// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "P7_Character.generated.h"
class UCapsuleComponent; 
class USpringArmComponent; 
class UCameraComponent; 
class USkeletalMeshComponent;
struct FInputActionValue;
UCLASS()
class PROBLEM7_API AP7_Character : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AP7_Character();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components") 
	UCapsuleComponent* CapsuleComp; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components") 
	USkeletalMeshComponent* SkeletalMeshComp; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components") 
	USpringArmComponent* SpringArmComp; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components") 
	UCameraComponent* CameraComp;
	float Gravity_V;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	float ConstGravity=0.98f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	float TerminalVelocity=0.2f;//종단속도 중력가속도가 무한으로 증가하지만 현실에서는 공기저항으로 특정 속도에서 더 이상 가속하지 않기 때문에 임의 숫자로 종단속도 설정 
	UPROPERTY(VIsibleAnywhere, BlueprintReadOnly, Category = "Platform")
	float MoveSpeed=1.0f;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& value);
	void Fly(const FInputActionValue& value);
	void Look(const FInputActionValue& value);
	void Roll(const FInputActionValue& value);
};
