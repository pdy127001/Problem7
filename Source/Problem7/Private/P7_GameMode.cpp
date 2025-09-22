// Fill out your copyright notice in the Description page of Project Settings.


#include "P7_GameMode.h"
#include "P7_Character.h"
#include "P7_PlayerController.h"

AP7_GameMode::AP7_GameMode() 
{
	DefaultPawnClass = AP7_Character::StaticClass();
	PlayerControllerClass = AP7_PlayerController::StaticClass();
}
