#include "PaperPlane.h"
#include "EngineUtils.h"           // TActorIterator
#include "DrawDebugHelpers.h"
#include "Engine/EngineTypes.h"

APaperPlane::APaperPlane()
{
    PrimaryActorTick.bCanEverTick = true;

    // 컴포넌트
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    StaticMeshComp->SetupAttachment(SceneRoot);

    // 실제 충돌 없음(모두 Ignore) → 멈춤 방지
    StaticMeshComp->SetSimulatePhysics(false);
    StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    StaticMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);

    Velocity = FVector(1, 0, 0) * 200.f;
}

void APaperPlane::BeginPlay()
{
    Super::BeginPlay();

    PhaseOffsetRad = FMath::FRandRange(0.f, 2.f * PI);
    ActualOrbitRadius = OrbitRadius;

    InitializeOnOrbit();
}

void APaperPlane::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    const FVector DesiredVel = ComputeDesiredVelocity(DeltaTime);
    const FVector DesiredDir = DesiredVel.IsNearlyZero() ? GetActorForwardVector() : DesiredVel.GetSafeNormal();

    const FVector Avoid = ComputeAvoidanceSteer(DesiredDir);
    const FVector Sep3D = ComputePersonalSpace3D();
    const FVector Coh = ComputeCohesionSteer();
    const FVector AlignV = ComputeAlignmentSteer();

    // 1) 코어(궤도/분리/응집/정렬/회피)만 먼저 합성
    FVector Core = DesiredVel + Avoid + Sep3D + Coh + AlignV;
    FVector Dir = Core.IsNearlyZero() ? DesiredDir : Core.GetSafeNormal();
    float   TargetSpeed = FMath::Max(CruiseSpeed, Core.Size());

    // 2) Wiggle(간단 인라인) 생성
    static float WiggleTime = 0.f;
    WiggleTime += DeltaTime * 2.0f;                 // 꿈틀 속도(1.2~2.2로 조절 가능)
    const float Seed = (GetUniqueID() % 997) * 0.013f;

    const float nx = FMath::PerlinNoise1D(WiggleTime + Seed);
    const float ny = FMath::PerlinNoise1D(WiggleTime + Seed + 37.21f);

    const FVector Right = FVector::CrossProduct(FVector::UpVector, Dir).GetSafeNormal();
    const FVector Up = FVector::UpVector;

    FVector Wiggle = (Right * nx + Up * ny);
    if (!Wiggle.IsNearlyZero())
    {
        Wiggle = Wiggle.GetSafeNormal() * 800.f;    // 꿈틀 세기(80~160 권장)
    }

    // 3) 진행방향에 수직인 성분만 남겨 정규화에 씻기지 않게 유지
    FVector WiggleLat = Wiggle - FVector::DotProduct(Wiggle, Dir) * Dir;
    // 과도 억제: 목표속도의 30% 한도
    WiggleLat = WiggleLat.GetClampedToMaxSize(0.6f * TargetSpeed);

    // 4) 최종 타깃 속도 = 방향*속도 + 측면 Wiggle
    FVector TargetVel = Dir * TargetSpeed + WiggleLat;

    // 5) 보간/감쇠/상한
    Velocity = FMath::VInterpTo(Velocity, TargetVel, DeltaTime, 1.6f);
    Velocity *= FMath::Clamp(Damping, 0.f, 1.f);
    Velocity = Velocity.GetClampedToMaxSize(MaxSpeed);

    ApplyMovementAndRotation(DeltaTime);

    if (bDrawDebug)
    {
        const FVector C = GetCenter();
        DrawDebugCircle(GetWorld(), C, ActualOrbitRadius, 64, FColor::Blue, false, 0.f, 0, 1.0f,
            FVector(1, 0, 0), FVector(0, 1, 0), false);
        DrawDebugLine(GetWorld(), GetActorLocation(),
            GetActorLocation() + Velocity.GetSafeNormal() * 200.f,
            FColor::Green, false, 0.f, 0, 2.f);
    }
}

FVector APaperPlane::GetCenter() const
{
    return CenterActor ? CenterActor->GetActorLocation() : ManualCenter;
}

void APaperPlane::InitializeOnOrbit()
{
    const FVector C = GetCenter();

    const FVector RadialDir = FVector(FMath::Cos(PhaseOffsetRad), FMath::Sin(PhaseOffsetRad), 0.f);
    const FVector Pos2D = C + RadialDir * ActualOrbitRadius;

    const float desiredZ = C.Z + HeightOffset;
    SetActorLocation(FVector(Pos2D.X, Pos2D.Y, desiredZ));

    FVector T = FVector::CrossProduct(FVector::UpVector, RadialDir).GetSafeNormal();
    if (bClockwise) T *= -1.f;
    Velocity = T * CruiseSpeed;
}

FVector APaperPlane::ComputeDesiredVelocity(float /*DeltaTime*/) const
{
    const FVector C = GetCenter();

    const FVector ToMe = GetActorLocation() - C;
    FVector R2D(ToMe.X, ToMe.Y, 0.f);
    float Dist2D = R2D.Size();
    if (Dist2D < 1.f) { R2D = FVector(1, 0, 0); Dist2D = 1.f; }

    FVector T = FVector::CrossProduct(FVector::UpVector, R2D).GetSafeNormal();
    if (bClockwise) T *= -1.f;

    const float   RadError = Dist2D - ActualOrbitRadius;
    const FVector RadDir = R2D / Dist2D;
    const FVector RadCorrVel = (-RadError * RadiusGain) * RadDir;

    const float desiredZ = C.Z + HeightOffset;
    const float zErr = GetActorLocation().Z - desiredZ;
    float zCorr = (-zErr) * HeightGain;
    zCorr = FMath::Clamp(zCorr, -MaxZCorrection, MaxZCorrection);

    FVector DesiredVel = (T * CruiseSpeed) + RadCorrVel;
    DesiredVel.Z = zCorr;
    return DesiredVel;
}

FVector APaperPlane::ComputeAvoidanceSteer(const FVector& DesiredDir) const
{
    // 모든 액터 대상: WorldStatic / WorldDynamic / Pawn / PhysicsBody
    FCollisionObjectQueryParams ObjQuery;
    ObjQuery.AddObjectTypesToQuery(ECC_WorldStatic);
    ObjQuery.AddObjectTypesToQuery(ECC_WorldDynamic);
    ObjQuery.AddObjectTypesToQuery(ECC_Pawn);
    ObjQuery.AddObjectTypesToQuery(ECC_PhysicsBody);

    const FVector Start = GetActorLocation();
    const FVector Fwd = DesiredDir.GetSafeNormal();
    const FVector Right = FVector::CrossProduct(FVector::UpVector, Fwd).GetSafeNormal();
    const FVector Left = -Right;

    const FVector DirF = Fwd;
    const FVector DirL = FQuat(FVector::UpVector, FMath::DegreesToRadians(+SideCheckAngleDeg)) * Fwd;
    const FVector DirR = FQuat(FVector::UpVector, FMath::DegreesToRadians(-SideCheckAngleDeg)) * Fwd;

    FVector DirU = Fwd, DirD = Fwd;
    if (bAllowVerticalAvoidance)
    {
        const FVector AxisRight = Right;
        DirU = FQuat(AxisRight, FMath::DegreesToRadians(+VerticalCheckAngleDeg)) * Fwd;
        DirD = FQuat(AxisRight, FMath::DegreesToRadians(-VerticalCheckAngleDeg)) * Fwd;
    }

    const FCollisionShape Sphere = FCollisionShape::MakeSphere(AvoidProbeRadius);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(PlaneAvoidStrong), false);
    Params.AddIgnoredActor(this);

    auto Probe = [&](const FVector& Dir, FVector& OutNearestNormal)->float
        {
            TArray<FHitResult> Hits;
            const FVector End = Start + Dir * AvoidTraceDistance;
            const bool bHit = GetWorld()->SweepMultiByObjectType(
                Hits, Start, End, FQuat::Identity, ObjQuery, Sphere, Params);

            if (!bHit) return AvoidTraceDistance;

            float bestDist = AvoidTraceDistance;
            FVector bestNorm = FVector::ZeroVector;

            for (const FHitResult& H : Hits)
            {
                if (!H.GetActor() || H.GetActor() == this) continue;

                // 옵션: 회피에서 PaperPlane을 장애물로 볼지 여부
                if (!bAvoidOtherPlanesInAvoidance && H.GetActor()->IsA(APaperPlane::StaticClass())) continue;

                if (H.Distance > 0.f && H.Distance < bestDist)
                {
                    bestDist = H.Distance;
                    bestNorm = H.ImpactNormal;
                }
            }

            OutNearestNormal = bestNorm;
            return bestDist;
        };

    // 각 방향 스코어
    FVector Nf, Nr, Nl, Nu, Nd;
    const float Sf = Probe(DirF, Nf);
    const float Sl = Probe(DirL, Nl);
    const float Sr = Probe(DirR, Nr);
    float Su = AvoidTraceDistance, Sd = AvoidTraceDistance;
    if (bAllowVerticalAvoidance)
    {
        Su = Probe(DirU, Nu);
        Sd = Probe(DirD, Nd);
    }

    const float f = FMath::Clamp(1.f - (Sf / AvoidTraceDistance), 0.f, 1.f); // 0 안전 ~ 1 매우 가까움

    FVector NormalPush = FVector::ZeroVector;
    if (!Nf.IsNearlyZero()) NormalPush += Nf.GetSafeNormal() * (AvoidNormalWeight * f);

    const bool  bLeftSafer = (Sl > Sr);
    const float sideGain = FMath::Abs(Sl - Sr) / AvoidTraceDistance;
    const FVector SideDir = bLeftSafer ? Left : Right;
    FVector SidePush = SideDir * (AvoidSideWeight * (f + sideGain));

    FVector VertPush = FVector::ZeroVector;
    if (bAllowVerticalAvoidance)
    {
        const bool bUpSafer = (Su >= Sd);
        const float vertGain = FMath::Abs(Su - Sd) / AvoidTraceDistance;
        const FVector UpDir = (FVector::UpVector + Fwd * 0.2f).GetSafeNormal();
        const FVector DownDir = (-FVector::UpVector + Fwd * 0.2f).GetSafeNormal();
        VertPush = (bUpSafer ? UpDir : DownDir) * (0.6f * (f + vertGain));
    }

    const FVector Brake = -Fwd * (BrakingStrength * f);

    FVector Avoid = (NormalPush + SidePush + VertPush + Brake);
    if (!Avoid.IsNearlyZero())
    {
        Avoid = Avoid.GetClampedToMaxSize(MaxSpeed) * AvoidAggressiveness;
    }

    // 회피 중 최소 속도 보장
    const float speed = Velocity.Size();
    if (f > 0.2f && speed < MinSpeedDuringAvoid)
    {
        Avoid += Fwd * (MinSpeedDuringAvoid - speed);
    }

    if (bDrawDebug)
    {
        auto DrawProbe = [&](const FVector& Dir, float S, const FColor& Col)
            {
                DrawDebugLine(GetWorld(), Start, Start + Dir * FMath::Min(S, AvoidTraceDistance), Col, false, 0.f, 0, 1.5f);
            };
        DrawProbe(DirF, Sf, FColor::Red);
        DrawProbe(DirL, Sl, FColor::Orange);
        DrawProbe(DirR, Sr, FColor::Orange);
        if (bAllowVerticalAvoidance)
        {
            DrawProbe(DirU, Su, FColor::Yellow);
            DrawProbe(DirD, Sd, FColor::Yellow);
        }
    }

    return Avoid;
}

FVector APaperPlane::ComputePersonalSpace3D() const
{
    if (!bEnablePersonalSpace3D || PersonalSpaceRadius <= 1.f)
        return FVector::ZeroVector;

    const FVector MyLoc = GetActorLocation();
    const float   R = PersonalSpaceRadius;

    FVector Sum = FVector::ZeroVector;

    for (TActorIterator<APaperPlane> It(GetWorld()); It; ++It)
    {
        APaperPlane* Other = *It;
        if (!Other || Other == this) continue;

        const FVector D = Other->GetActorLocation() - MyLoc; // 이웃→나
        const float   Dist = D.Size();
        if (Dist > KINDA_SMALL_NUMBER && Dist < R)
        {
            const float w = 1.f - (Dist / R);   // 가까울수록 큼
            Sum += (-D / Dist) * w;             // 기본 반발 누적(등방)
        }
    }

    if (Sum.IsNearlyZero()) return FVector::ZeroVector;

    // 축별 가중치 적용(비등방)
    FVector Weighted(
        Sum.X * PersonalSpaceAxisWeight.X,
        Sum.Y * PersonalSpaceAxisWeight.Y,
        Sum.Z * PersonalSpaceAxisWeight.Z
    );

    if (Weighted.IsNearlyZero()) return FVector::ZeroVector;

    return Weighted.GetSafeNormal() * PersonalSpaceStrength;
}

FVector APaperPlane::ComputeCohesionSteer() const
{
    if (!bUseCohesion || CohesionRadius <= 10.f) return FVector::ZeroVector;

    const FVector Me = GetActorLocation();
    FVector SumPos = FVector::ZeroVector;
    int32   Cnt = 0;

    for (TActorIterator<APaperPlane> It(GetWorld()); It; ++It)
    {
        const APaperPlane* Other = *It;
        if (!Other || Other == this) continue;

        const float d = FVector::Dist(Me, Other->GetActorLocation());
        if (d <= CohesionRadius)
        {
            SumPos += Other->GetActorLocation();
            ++Cnt;
        }
    }
    if (Cnt == 0) return FVector::ZeroVector;

    const FVector COM = SumPos / Cnt;
    FVector toCOM = (COM - Me);
    if (toCOM.IsNearlyZero()) return FVector::ZeroVector;

    // 축별 가중치(비등방 응집)
    toCOM = FVector(toCOM.X * CohesionAxisWeight.X,
        toCOM.Y * CohesionAxisWeight.Y,
        toCOM.Z * CohesionAxisWeight.Z);

    const float dist = toCOM.Size();
    const FVector dir = toCOM / FMath::Max(dist, KINDA_SMALL_NUMBER);

    // 멀수록 조금 더 강하게(0~1)
    const float gain = FMath::Clamp(dist / CohesionRadius, 0.f, 1.f);

    return dir * (CohesionStrength * gain);
}

FVector APaperPlane::ComputeAlignmentSteer() const
{
    if (!bUseAlignment || AlignmentRadius <= 10.f) return FVector::ZeroVector;

    const FVector Me = GetActorLocation();
    FVector SumVel = FVector::ZeroVector;
    int32   Cnt = 0;

    for (TActorIterator<APaperPlane> It(GetWorld()); It; ++It)
    {
        const APaperPlane* Other = *It;
        if (!Other || Other == this) continue;

        const float d = FVector::Dist(Me, Other->GetActorLocation());
        if (d <= AlignmentRadius)
        {
            SumVel += Other->Velocity;
            ++Cnt;
        }
    }
    if (Cnt == 0) return FVector::ZeroVector;

    const FVector Avg = SumVel.IsNearlyZero() ? FVector::ZeroVector : SumVel.GetSafeNormal();
    if (Avg.IsNearlyZero()) return FVector::ZeroVector;

    return Avg * AlignmentStrength;
}

void APaperPlane::ApplyMovementAndRotation(float DeltaTime)
{
    // 실제 충돌 없음(모두 Ignore) → 스윕 없이 위치만 갱신
    AddActorWorldOffset(Velocity * DeltaTime, /*bSweep=*/false);

    if (!Velocity.IsNearlyZero())
    {
        const FRotator Target = Velocity.Rotation();
        const FRotator NewRot = FMath::RInterpTo(GetActorRotation(), Target, DeltaTime, AlignTurnSpeed);

        if (bAlignPitchToVelocity)
            SetActorRotation(FRotator(NewRot.Pitch, NewRot.Yaw, 0.f));
        else
            SetActorRotation(FRotator(0.f, NewRot.Yaw, 0.f));
    }
}
