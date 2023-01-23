// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/AudioComponent.h"
#include "VRController.h"
#include "VRCharacter.generated.h"

//Controla el estatus del Character
USTRUCT()
struct FCharacterStatus {

	GENERATED_BODY()

		bool resting = 1; // 0
	bool walking = 0; // 1
	bool running = 0; // 2
	bool jumping = 0; // 3
	bool onground = 1; //4
	bool climbing = 0;
	bool scaling = 0;
};

UCLASS()
class VR_ENGLISH_API AVRCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVRCharacter();
	void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	// Área de Vista
private:
	void StopSoundEffect(int IndexAudio);
	void PlaySoundEffect(int IndexAudio, int soundVar);

private:
	int WalkingVarSound;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Audio, meta = (AllowPrivateAccess = "true"))
		class USoundCue* CharacterSoundCue;

	UAudioComponent* WalkingAudioComponent;
	UAudioComponent* JumpingAudioComponent;
	UAudioComponent* CrouchingAudioComponent;
	UAudioComponent* ClimbingAudioComponent;


	// Área de Modelo
private:
	FCharacterStatus Status; // Controla el status del character
	UPROPERTY()
		UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	float WalkSpeed = CharMovement->MaxWalkSpeed; // Velocidad de caminado

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<AVRController> VRControllerClass;
public:
	void SetStatus(int option, bool status);
	bool GetStatus(int option);
	double FindVectorZControlLeft();
	double FindVectorZControlRight();
	double FindVectorZHeadPlayer();
	bool StateLeft;
	bool StateRight;

	FVector LocationRightC;
	FVector LocationLeftC;
	FVector PlayerLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Crouch)
		FVector CrouchEyeOffset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Crouch)
		float CrouchSpeed;

	// Área de Controlador

private:
	void Jumping();
	void StopJump();
	void MoveForward(float Throttle);
	void MoveRight(float Throttle);
	void Running(float Throttle);
	void CrouchControl();

	virtual void Landed(const FHitResult& Hit) override;
	void GripLeft();
	void GripRight();
	void ReleaseLeft();
	void ReleaseRight();
	void ModelController();
	void BestCameraView();



public:
	UPROPERTY(VisibleAnywhere)
		class UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere)
		class USceneComponent* VRRoot;
	UPROPERTY(VisibleAnywhere)
		AVRController* LeftController;
	UPROPERTY(VisibleAnywhere)
		AVRController* RightController;


private:
	UPROPERTY(EditAnywhere)
		float JumpTime = 1.5;
	bool RunningStarting = true; // Variable to DoOnce in running
	bool RunningReinit = true; // Variable to DoOnce in running


};
