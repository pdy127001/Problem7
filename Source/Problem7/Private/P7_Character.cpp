// Fill out your copyright notice in the Description page of Project Settings.


#include "P7_Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h" 
#include "GameFramework/SpringArmComponent.h" 
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "P7_PlayerController.h"
// Sets default values
AP7_Character::AP7_Character()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp")); 
	CapsuleComp->SetSimulatePhysics(false);
	SetRootComponent(CapsuleComp); 
	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh")); 
	SkeletalMeshComp->SetupAttachment(CapsuleComp);
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/Resources/Characters/Meshes/SKM_Manny.SKM_Manny"));
	if (MeshAsset.Succeeded())
	{
		SkeletalMeshComp->SetSkeletalMesh(MeshAsset.Object);
	}
	SkeletalMeshComp->SetSimulatePhysics(false);
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm")); 
	SpringArmComp->SetupAttachment(CapsuleComp); 
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera")); 
	CameraComp->SetupAttachment(SpringArmComp);
	Gravity_V = 0;
}

// Called when the game starts or when spawned
void AP7_Character::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AP7_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0, 0, 100);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this); 

	bool bOnGround = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Visibility,
		Params
	);

	if (bOnGround)
	{
		Gravity_V = 0;
		MoveSpeed = 1.0f;
	}
	else
	{
		if (TerminalVelocity >= Gravity_V*-1.0f)
		{
			Gravity_V -= ConstGravity * DeltaTime;
			MoveSpeed = 0.5f;
		}
		AddActorWorldOffset(FVector(0, 0, 1) * Gravity_V, true);
	}
}

// Called to bind functionality to input
void AP7_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AP7_PlayerController* PlayerController = Cast<AP7_PlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&AP7_Character::Move
				);
			}
			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&AP7_Character::Look
				);
			}
			if (PlayerController->FlyAction)
			{
				EnhancedInput->BindAction(
					PlayerController->FlyAction,
					ETriggerEvent::Triggered,
					this,
					&AP7_Character::Fly
				);
			}
			if (PlayerController->RollAction)
			{
				EnhancedInput->BindAction(
					PlayerController->RollAction,
					ETriggerEvent::Triggered,
					this,
					&AP7_Character::Roll
				);
			}
		}
	}
}

void AP7_Character::Move(const FInputActionValue& value) 
{
	if (!Controller) return;
	const FVector2D MoveInput = value.Get<FVector2D>();
	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddActorLocalOffset(FVector(1,0,0)* 5 * MoveInput.X*MoveSpeed, true);//false는 장애물통과, true는 충돌
	}
	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddActorWorldOffset(GetActorRightVector()*5* MoveInput.Y*MoveSpeed, true);
	}
}
void AP7_Character::Fly(const FInputActionValue& value)
{
	if (!Controller) return;
	const float FlyInput = value.Get<float>();
	if (!FMath::IsNearlyZero(FlyInput))
	{
		AddActorLocalOffset(FVector(0,0,1) * 5 * FlyInput, true);
	}
}
void AP7_Character::Look(const FInputActionValue& value)
{
	float Delta = GetWorld()->GetDeltaSeconds();
	if (!Controller) return;
	const FVector2D LookInput = value.Get<FVector2D>();
	FRotator LookRot = FRotator(LookInput.Y, LookInput.X,0 );
	AddActorLocalRotation(LookRot, true);
	
}
void AP7_Character::Roll(const FInputActionValue& value)
{
	if (!Controller) return;
	const float RollInput = value.Get<float>();
	FRotator RollRot = FRotator(0, 0, RollInput * 10);
	if (!FMath::IsNearlyZero(RollRot.Roll))
	{
		AddActorLocalRotation(RollRot, true);
	}
}