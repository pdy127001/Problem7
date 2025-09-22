#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaperPlane.generated.h"

/**
 * ���˵� ���� + ��� ���� ��� ȸ��(���� ����, ���� �浹 ����)
 * + 3D �۽��� �����̽�(�ະ ����) + Cohesion(�̿� �߽� ����) + Alignment(���� ����)
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
    // ���� ����
    FVector GetCenter() const;
    FVector ComputeDesiredVelocity(float DeltaTime) const;           // ���� + ������/�� ����
    FVector ComputeAvoidanceSteer(const FVector& DesiredDir) const; // ��� ���� ��� ����(ȸ�� + ����)
    FVector ComputePersonalSpace3D() const;                          // 3D �и�(�ະ ����)
    FVector ComputeCohesionSteer() const;                            // �̿� �߽����� ����(�ະ ����)
    FVector ComputeAlignmentSteer() const;                           // �̿� ��� �������� ����
    void    ApplyMovementAndRotation(float DeltaTime);               // �̵� + ȸ��
    void    InitializeOnOrbit();                                     // ���� ����/�ӵ�

public:
    // ���� ������Ʈ ����������������������������������������������������������������������������������������������
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* SceneRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* StaticMeshComp;

    // ���� Ʈ��(��) ����������������������������������������������������������������������������������������������
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
    float MaxZCorrection = 600.f; // �� ���� ����

    // ���� ���� ȸ��(��� ���� ���) ����������������������������������������������������������
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    float AvoidTraceDistance = 800.f;     // ���� �Ÿ�

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    float AvoidProbeRadius = 45.f;        // ���� ������

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    float SideCheckAngleDeg = 45.f;       // ��/��

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    float VerticalCheckAngleDeg = 20.f;   // ��/�� �밢

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    bool bAllowVerticalAvoidance = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    float AvoidAggressiveness = 1.4f;     // ȸ�� ��ü ������

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    float AvoidNormalWeight = 1.0f;       // ��Ʈ ��� ������ �б�

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    float AvoidSideWeight = 1.0f;         // ��/�� ������ ������ ��Ƽ��

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    float BrakingStrength = 900.f;        // ������ ������ ������ ����

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    float MinSpeedDuringAvoid = 250.f;    // ȸ�� �� �ּ� ���� �ӵ�

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoid")
    bool bAvoidOtherPlanesInAvoidance = false; // true�� ȸ�� �������� PaperPlane�� ��ֹ��� ���

    // ���� 3D �۽��� �����̽�(�ະ ���) ������������������������������������������������
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing")
    bool bEnablePersonalSpace3D = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing", meta = (ClampMin = "10.0", ClampMax = "2000.0"))
    float PersonalSpaceRadius = 180.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing", meta = (ClampMin = "10.0", ClampMax = "5000.0"))
    float PersonalSpaceStrength = 280.f;

    // �ະ ����ġ (X/Y/Z ���� ����)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing")
    FVector PersonalSpaceAxisWeight = FVector(1.0f, 1.0f, 1.0f);

    // ���� Cohesion(����: �̿� �߽����� ����) ����������������������������������������
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cohesion")
    bool bUseCohesion = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cohesion", meta = (ClampMin = "50.0", ClampMax = "5000.0"))
    float CohesionRadius = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cohesion", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
    float CohesionStrength = 450.f;

    // �ະ ����ġ: X�� Ű��� ���������� ���� �� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cohesion")
    FVector CohesionAxisWeight = FVector(1.5f, 1.0f, 0.95f);

    // ���� Alignment(����: ���� ���߱�) ��������������������������������������������������
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alignment")
    bool bUseAlignment = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alignment", meta = (ClampMin = "50.0", ClampMax = "5000.0"))
    float AlignmentRadius = 700.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alignment", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
    float AlignmentStrength = 360.f;

    // ���� �ӵ�/ȸ��/���� ����������������������������������������������������������������������������������
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

    // �����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawDebug = false;

private:
    float PhaseOffsetRad = 0.f;
    float ActualOrbitRadius = 0.f;
};
