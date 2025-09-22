#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaperPlane.generated.h"

/**
 * 원궤도 비행 + 모든 액터 대상 회피(스윕 감지, 실제 충돌 없음)
 * + 3D 퍼스널 스페이스(축별 가중) + Cohesion(이웃 중심 끌림) + Alignment(방향 정렬)
 */
UCLASS()
class PROBLEM7_API APaperPlane : public AActor
{
    GENERATED_BODY()

public:
    APaperPlane();
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

private:
    // 내부 로직
    FVector GetCenter() const;
    FVector ComputeDesiredVelocity(float DeltaTime) const;           // 접선 + 반지름/고도 보정
    FVector ComputeAvoidanceSteer(const FVector& DesiredDir) const; // 모든 액터 대상 스윕(회피 + 감속)
    FVector ComputePersonalSpace3D() const;                          // 3D 분리(축별 가중)
    FVector ComputeCohesionSteer() const;                            // 이웃 중심으로 끌림(축별 가중)
    FVector ComputeAlignmentSteer() const;                           // 이웃 평균 방향으로 정렬
    void    ApplyMovementAndRotation(float DeltaTime);               // 이동 + 회전
    void    InitializeOnOrbit();                                     // 시작 스냅/속도

public:
    // ── 컴포넌트 ───────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* SceneRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* StaticMeshComp;

    // ── 트랙(원) ───────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    AActor* CenterActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    FVector ManualCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    bool bClockwise = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track", meta = (ClampMin = "100.0"))
    float OrbitRadius = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track", meta = (ClampMin = "50.0"))
    float CruiseSpeed = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    float RadiusGain = 2.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    float HeightOffset = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    float HeightGain = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track", meta = (ClampMin = "0.0"))
    float MaxZCorrection = 600.f; // 고도 보정 상한

    // ── 강한 회피(모든 액터 대상) ─────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    float AvoidTraceDistance = 800.f;     // 감지 거리

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    float AvoidProbeRadius = 45.f;        // 스윕 반지름

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    float SideCheckAngleDeg = 45.f;       // 좌/우

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    float VerticalCheckAngleDeg = 20.f;   // 상/하 대각

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    bool bAllowVerticalAvoidance = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    float AvoidAggressiveness = 1.4f;     // 회피 전체 스케일

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    float AvoidNormalWeight = 1.0f;       // 히트 노멀 쪽으로 밀기

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    float AvoidSideWeight = 1.0f;         // 좌/우 안전한 쪽으로 스티어

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    float BrakingStrength = 900.f;        // 정면이 가깝게 막히면 감속

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    float MinSpeedDuringAvoid = 250.f;    // 회피 중 최소 유지 속도

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    bool bAvoidOtherPlanesInAvoidance = false; // true면 회피 스윕에서 PaperPlane도 장애물로 취급

    // ── 3D 퍼스널 스페이스(축별 계수) ────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing")
    bool bEnablePersonalSpace3D = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing", meta = (ClampMin = "10.0", ClampMax = "2000.0"))
    float PersonalSpaceRadius = 180.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing", meta = (ClampMin = "10.0", ClampMax = "5000.0"))
    float PersonalSpaceStrength = 280.f;

    // 축별 가중치 (X/Y/Z 각각 조절)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing")
    FVector PersonalSpaceAxisWeight = FVector(1.0f, 1.0f, 1.0f);

    // ── Cohesion(응집: 이웃 중심으로 끌림) ────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cohesion")
    bool bUseCohesion = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cohesion", meta = (ClampMin = "50.0", ClampMax = "5000.0"))
    float CohesionRadius = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cohesion", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
    float CohesionStrength = 450.f;

    // 축별 가중치: X를 키우면 진행축으로 서로 더 끌림
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cohesion")
    FVector CohesionAxisWeight = FVector(1.5f, 1.0f, 0.95f);

    // ── Alignment(정렬: 방향 맞추기) ─────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alignment")
    bool bUseAlignment = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alignment", meta = (ClampMin = "50.0", ClampMax = "5000.0"))
    float AlignmentRadius = 700.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alignment", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
    float AlignmentStrength = 360.f;

    // ── 속도/회전/감쇠 ─────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Damping = 0.99f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float MaxSpeed = 900.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float AlignTurnSpeed = 6.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bAlignPitchToVelocity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector Velocity = FVector::ZeroVector;

    // 디버그
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawDebug = false;

private:
    float PhaseOffsetRad = 0.f;
    float ActualOrbitRadius = 0.f;
};
