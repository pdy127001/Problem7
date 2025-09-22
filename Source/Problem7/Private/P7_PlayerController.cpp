// Fill out your copyright notice in the Description page of Project Settings.


#include "P7_PlayerController.h"
#include "EnhancedInputSubsystems.h"
AP7_PlayerController::AP7_PlayerController() 
	:InputMappingContext(nullptr),
	MoveAction(nullptr),
	LookAction(nullptr),
	FlyAction(nullptr),
	RollAction(nullptr)
{
	/*static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMCAsset(TEXT("/Game/Inputs/IMC_Character.IMC_Character")
	);
	if (IMCAsset.Succeeded())
	{
		InputMappingContext = IMCAsset.Object.Get();
	}*/
	//IA는 단순 UObject Asset → FObjectFinder가 아직 잘 동작,IMC는 DataAsset → FObjectFinder에서 잘 안 됨 형태가 달라서라고 함
	static ConstructorHelpers::FObjectFinder<UInputAction> MAAsset(TEXT("/Game/Inputs/IA_Move.IA_Move"));
	if (MAAsset.Succeeded())
	{
		MoveAction = MAAsset.Object.Get();
	}
	static ConstructorHelpers::FObjectFinder<UInputAction> FAAsset(TEXT("/Game/Inputs/IA_Fly.IA_Fly"));
	if (FAAsset.Succeeded())
	{
		FlyAction = FAAsset.Object.Get();
	}
	static ConstructorHelpers::FObjectFinder<UInputAction> LAAsset(TEXT("/Game/Inputs/IA_Look.IA_Look"));
	if (LAAsset.Succeeded())
	{
		LookAction = LAAsset.Object.Get();
	}
	static ConstructorHelpers::FObjectFinder<UInputAction> RAAsset(TEXT("/Game/Inputs/IA_Roll.IA_Roll"));
	if (RAAsset.Succeeded())
	{
		RollAction = RAAsset.Object.Get();
	}
}
void AP7_PlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}
}