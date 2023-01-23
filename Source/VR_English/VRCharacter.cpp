// Fill out your copyright notice in the Description page of Project Settings.
#include "VRCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "MotionControllerComponent.h"
#include "NavigationSystem.h"


// Controls character status, starts as resting


// Sets default values
AVRCharacter::AVRCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//Send true CanCrouch
	this->CharMovement->GetNavAgentPropertiesRef().bCanCrouch = true;


	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	VRRoot->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(VRRoot);

	CrouchEyeOffset = FVector(0.f);
	CrouchSpeed = 6.f;





	// Init Audio Resource
	static ConstructorHelpers::FObjectFinder<USoundCue> CharacterSoundCueObject(TEXT("/ Script / Engine.SoundCue'/Game/Sound/SoundEffects/SC_CharacterActions.SC_CharacterActions'"));

	if (CharacterSoundCueObject.Succeeded())
	{
		CharacterSoundCue = CharacterSoundCueObject.Object;

		WalkingAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WalkingAudioComponent"));
		WalkingAudioComponent->SetupAttachment(RootComponent);

		JumpingAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("JumpingAudioComponent"));
		JumpingAudioComponent->SetupAttachment(RootComponent);

		CrouchingAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("CrouchingAudioComponent"));
		CrouchingAudioComponent->SetupAttachment(RootComponent);

		ClimbingAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ClimbingAudioComponent"));
		ClimbingAudioComponent->SetupAttachment(RootComponent);
	}

}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();
	this->JumpMaxHoldTime = JumpTime;

	ModelController();

	if (CharacterSoundCue) // Setting sound to different audio components, everything comes from CharacterSoundCue in Content Browser
	{
		if (WalkingAudioComponent)
			WalkingAudioComponent->SetSound(CharacterSoundCue);

		if (JumpingAudioComponent)
			JumpingAudioComponent->SetSound(CharacterSoundCue);

		if (CrouchingAudioComponent)
			CrouchingAudioComponent->SetSound(CharacterSoundCue);

		if (ClimbingAudioComponent)
			ClimbingAudioComponent->SetSound(CharacterSoundCue);
	}


}


// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT(".............. %d"), int( GetCharacterMovement()->IsMovingOnGround() ) ));
	float CrouchInterpTime = FMath::Min(5.f, CrouchSpeed * DeltaTime);	//Do a calcution with DeltaTime to get the speed
	CrouchEyeOffset = (5.f - CrouchInterpTime) * CrouchEyeOffset;		//Set the speed of the crouch


	BestCameraView();
	FindVectorZControlLeft();
	FindVectorZControlRight();
	FindVectorZHeadPlayer();
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString::Printf(TEXT("Camera Location: %s"), ));

}

/*---------------------------------------------------------------VISTA------------------------------------------------------------------------------------------------*/

void AVRCharacter::PlaySoundEffect(int IndexAudio, int soundVar)
{
	/*
	IndexAudio
	0: Walking Effects (Walking, Running)
	1: Jumping Effects (Landing)
	*/

	if (CharacterSoundCue)
	{
		EAudioComponentPlayState WalkingPlayState = WalkingAudioComponent->GetPlayState(); // Is sound playing?
		EAudioComponentPlayState JumpingPlayState = JumpingAudioComponent->GetPlayState(); // Is sound playing?
		EAudioComponentPlayState CrouchingPlayState = CrouchingAudioComponent->GetPlayState(); // Is sound playing?
		EAudioComponentPlayState ClimbingPlayState = ClimbingAudioComponent->GetPlayState(); // Is sound playing?

		if (WalkingAudioComponent && IndexAudio == 0 && (WalkingPlayState != EAudioComponentPlayState::Playing))
		{
			WalkingAudioComponent->SetIntParameter(FName("SoundChanger"), soundVar);
			WalkingAudioComponent->Play(0.f);
		}

		if (JumpingAudioComponent && IndexAudio == 1 && (JumpingPlayState != EAudioComponentPlayState::Playing))
		{
			JumpingAudioComponent->SetIntParameter(FName("SoundChanger"), soundVar);
			JumpingAudioComponent->Play(0.f);
		}

		if (CrouchingAudioComponent && IndexAudio == 2 && (CrouchingPlayState != EAudioComponentPlayState::Playing))
		{
			CrouchingAudioComponent->SetIntParameter(FName("SoundChanger"), soundVar);
			CrouchingAudioComponent->Play(0.f);
		}

		if (ClimbingAudioComponent && IndexAudio == 3 && (ClimbingPlayState != EAudioComponentPlayState::Playing))
		{
			ClimbingAudioComponent->SetIntParameter(FName("SoundChanger"), soundVar);
			ClimbingAudioComponent->Play(0.f);
		}
	}
}

void AVRCharacter::StopSoundEffect(int IndexAudio)
{
	EAudioComponentPlayState WalkingPlayState = WalkingAudioComponent->GetPlayState(); // Is sound playing?
	EAudioComponentPlayState JumpingPlayState = JumpingAudioComponent->GetPlayState(); // Is sound playing?
	EAudioComponentPlayState CrouchingPlayState = CrouchingAudioComponent->GetPlayState(); // Is sound playing?
	EAudioComponentPlayState ClimbingPlayState = ClimbingAudioComponent->GetPlayState(); // Is sound playing?


	if (CharacterSoundCue && WalkingAudioComponent) // IF sound is playing
	{
		if (WalkingAudioComponent && IndexAudio == 0 && (WalkingPlayState == EAudioComponentPlayState::Playing))
			WalkingAudioComponent->Stop();
		if (JumpingAudioComponent && IndexAudio == 1 && (JumpingPlayState == EAudioComponentPlayState::Playing))
			JumpingAudioComponent->Stop();
		if (CrouchingAudioComponent && IndexAudio == 2 && (CrouchingPlayState == EAudioComponentPlayState::Playing))
			CrouchingAudioComponent->Stop();
		if (ClimbingAudioComponent && IndexAudio == 3 && (ClimbingPlayState == EAudioComponentPlayState::Playing))
			ClimbingAudioComponent->Stop();
	}
}

void AVRCharacter::ModelController() {

	LeftController = GetWorld()->SpawnActor<AVRController>(VRControllerClass);
	if (LeftController != nullptr) {
		LeftController->AttachToComponent(VRRoot, FAttachmentTransformRules::KeepRelativeTransform);
		LeftController->SetOwner(this);
		LeftController->SetHand(EControllerHand::Left);
	}


	RightController = GetWorld()->SpawnActor<AVRController>(VRControllerClass);

	if (RightController != nullptr) {
		RightController->AttachToComponent(VRRoot, FAttachmentTransformRules::KeepRelativeTransform);
		RightController->SetOwner(this);
		RightController->SetHand(EControllerHand::Right);

	}

	LeftController->PairController(RightController);
}


/*---------------------------------------------------------------MODELO------------------------------------------------------------------------------------------------*/
void AVRCharacter::SetStatus(int option, bool status)
{
	switch (option)
	{
	case 0:
		Status.resting = status;
		break;

	case 1:
		Status.running = status;
		break;

	case 2:
		Status.jumping = status;
		break;
	case 3:
		Status.climbing = status;
		break;
	case 4:
		Status.scaling = status;
		break;

	}
}

bool AVRCharacter::GetStatus(int option)
{
	switch (option)
	{
	case 0:
		return Status.resting;

	case 1:
		return Status.running;

	case 2:
		return Status.jumping;

	case 3:
		return Status.climbing;
	case 4:
		return Status.scaling;

	default:
		return 0;
	}

}


//This function Gets the real location in Z of the Right controller
double AVRCharacter::FindVectorZControlRight() {

	FVector Actor = GetActorLocation();
	//Real Location for RightController
	LocationRightC = RightController->MotionController->GetComponentLocation() - Actor;
	return LocationRightC.Z;
}

//This function Gets the real location in Z of the left controller
double AVRCharacter::FindVectorZControlLeft() {
	FVector Actor = GetActorLocation();
	//Real Location for LeftController
	LocationLeftC = LeftController->GetActorLocation() - Actor;
	return LocationLeftC.Z;
}

//This function Gets the real location in Z of the Camera Actor
double AVRCharacter::FindVectorZHeadPlayer() {

	FVector Actor = GetActorLocation();
	//Real Location For Player
	PlayerLocation = Camera->GetComponentLocation() - Actor;
	return PlayerLocation.Z;
}


/*---------------------------------------------------------------CONTROLADOR------------------------------------------------------------------------------------------------*/

void AVRCharacter::Landed(const FHitResult& Hit) // Event that triggers when character lands after falling
{
	Super::Landed(Hit);
	PlaySoundEffect(1, 2);
}

void AVRCharacter::Jumping()
{
	if (!GetStatus(2) && !CharMovement->IsFalling() && CharMovement->IsMovingOnGround() && (GetStatus(0) || CharMovement->IsWalking() || GetStatus(1) || CharMovement->IsCrouching())) // IF no está en jumping AND está onground AND se encuentra resting OR walking OR running
	{
		if (CharMovement->IsCrouching()) {
			CrouchControl();
		}
		else {
			this->Jump();
			SetStatus(2, true); // Set jumping status as true
		}
	}
}

void AVRCharacter::StopJump()
{
	if (GetStatus(2)) // IF está en jumping
	{
		this->StopJumping();
		SetStatus(2, false); // Set jumping status as false
	}
}

void AVRCharacter::MoveForward(float Throttle)
{

	if (GetStatus(1)) // IF running is true
	{
		// Velocidad acelerada 2.5
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * 2.5;
		// Play sound effect with index 1 (See CharacterSoundCue in Content Browser)
		WalkingVarSound = 1;

	}
	else if (CharMovement->IsWalking() && !GetStatus(1))
	{
		// Velocidad normal
		CharMovement->MaxWalkSpeed = WalkSpeed;
		// Play sound effect with index 0 (See CharacterSoundCue in Content Browser)
		WalkingVarSound = 0;
	}

	if (Throttle < 0.08 && Throttle > -0.08) // No se mueve el joystick
	{
		SetStatus(0, true); // resting true
		StopSoundEffect(0);

	}
	else
	{
		AddMovementInput(Camera->GetForwardVector(), Throttle);
		PlaySoundEffect(0, WalkingVarSound);

	}

	if (!(CharMovement->IsMovingOnGround()))
		StopSoundEffect(0);


}

void AVRCharacter::MoveRight(float Throttle)
{

	if (Throttle < 0.08 && Throttle > -0.08) // No se mueve el joystick
	{
		SetStatus(0, true); // resting true
	}
	else
	{
		AddMovementInput(Camera->GetRightVector(), Throttle);
	}


}

void AVRCharacter::Running(float Throttle)
{

	if ((GetStatus(0) || CharMovement->IsWalking()) && CharMovement->IsMovingOnGround()) // IF está en resting OR walking AND onground
	{
		if (Throttle > 0.3 || Throttle < -0.3) // El joystick está siendo activado más del 30%
		{
			SetStatus(1, true); // running true
			if (CharMovement->IsCrouching()) {
				CrouchControl();
			}

			if (RunningStarting) //DO ONCE
			{
				StopSoundEffect(0);
				RunningStarting = false;
			}

			RunningReinit = true;
		}
		else { // running false
			SetStatus(1, false);
			RunningStarting = true;

			if (RunningReinit) //DO ONCE
			{
				StopSoundEffect(0);
				RunningReinit = false;
			}

		}
	}
}

void AVRCharacter::CrouchControl() {

	if (CharMovement->IsCrouching()) {
		this->UnCrouch();
		PlaySoundEffect(2, 4);
	}

	else if (((GetStatus(0) || CharMovement->IsWalking()) && CharMovement->IsMovingOnGround()) && !GetStatus(1)) {
		this->Crouch();
		PlaySoundEffect(2, 3);
	}

	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Agachado"));
}


//This function is call when player press the Left grip button
void AVRCharacter::GripLeft() {
	int Selector = LeftController->ObjectSelect();					//Gets the value depending  on what character is grabbing
	switch (Selector)
	{

	case 0:															//Case 0 if character grabbing a Climb Object
		StateLeft = LeftController->GripClimb();					//Call the GripClimb function in Controller.cpp			
		if (StateLeft) {											//if function returns true
			SetStatus(3, true);										//Update the status in climbing
			//sonido
			PlaySoundEffect(3, 5);									//Play sound efect
		}
		break;

	case 1:															//Case 1 if character grabbing a Scale Object
		StateLeft = LeftController->GripScaleClimb();				//Call the GripScaleClimb function in Controller.cpp	
		if (StateLeft) {
			SetStatus(4, true);
			PlaySoundEffect(3, 7);
		}
		break;

	case 2:
		LeftController->GripPickup();
		break;


	default:
		break;
	}

}

//This function is call when player press the Right grip button
void AVRCharacter::GripRight() {
	int Selector = RightController->ObjectSelect();				//Gets the value depending  on what character is grabbing
	switch (Selector)
	{
	case 0:														//Case 0 if character grabbing a Climb Object
		StateRight = RightController->GripClimb();				//Call the GripClimb function in Controller.cpp	
		if (StateRight) {										//if function returns true
			SetStatus(3, true);									//Update the status in climbing
			//sonido
			PlaySoundEffect(3, 5);								//Play sound efect
		}
		break;

	case 1:
		StateRight = RightController->GripScaleClimb();			//Case 1 if character grabbing a Scale Object
																//Call the GripScaleClimb function in Controller.cpp
		if (StateRight) {
			SetStatus(4, true);
			PlaySoundEffect(3, 7);
		}
		break;

	case 2:
		RightController->GripPickup();
		break;

	default:
		break;
	}

}


//This function is call when player Release the Left grip button
void AVRCharacter::ReleaseLeft() {
	int Selector = LeftController->ObjectSelect();										//Gets the value depending  on what character is releasing
	switch (Selector)
	{
	case 0:
		StateLeft = LeftController->ReleaseClimb();										//Call the ReleaseClimb function in Controller.cpp
		if (!StateLeft) {																//if function returns false
			if (CharMovement->IsFalling() && !GetStatus(4)) {							//And if character if Falling
				SetStatus(3, false);													//Update the character status in not climbing
				//sonido	
				PlaySoundEffect(3, 6);													//Play sound efect
			}

		}
		break;

	case 1:
		StateLeft = LeftController->ReleaseScale(FindVectorZControlLeft(), FindVectorZControlRight(), FindVectorZHeadPlayer()); //Call the ReleaseScale function in Controller.cpp
																																//And send the values to functions 
		if (!StateLeft) {
			if (CharMovement->IsFalling()) {
				SetStatus(4, false);
			}
		}
		break;
	default:
		break;
	}


}


//This function is call when player Release the Right grip button
void AVRCharacter::ReleaseRight() {
	int Selector = RightController->ObjectSelect();										//Gets the value depending  on what character is releasing

	switch (Selector)
	{
	case 0:
		StateRight = RightController->ReleaseClimb();									//Call the ReleaseClimb function in Controller.cpp
		if (!StateRight) {																//if function returns false
			if (CharMovement->IsFalling() && !GetStatus(4)) {							//And if character if Falling
				SetStatus(3, false);													//Update the character status in not climbing
				//sonido
				PlaySoundEffect(3, 6);													//Play sound efect
			}
		}
		break;

	case 1:
		StateRight = RightController->ReleaseScale(FindVectorZControlLeft(), FindVectorZControlRight(), FindVectorZHeadPlayer());				//Call the ReleaseScale function in Controller.cpp
																																				//And send the values to functions 
		if (!StateRight) {
			if (CharMovement->IsFalling()) {
				SetStatus(4, false);
			}
		}

		break;

	default:
		break;
	}





}

/*Best Camera effect when Crouch*/
void AVRCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) { //This is a defualt function for Unreal
																						//HalfHeightAdjust and ScaledHalfHeightAdjust are parameters that unreal send automatically.
																						//Depends on the View and Height of the player.
	if (HalfHeightAdjust == 0.f) return;

	float StartBaseEyeHeight = BaseEyeHeight;											//BaseEyeHeight is property for Characters in Unreal Engine
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);						//With these we can make sure that the class is going to call the function
	CrouchEyeOffset.Z += StartBaseEyeHeight - BaseEyeHeight + HalfHeightAdjust;			//Just modify the Vector  on Z
	Camera->SetRelativeLocation(FVector(BaseEyeHeight), false);				//Set the new BaseEyeHeight to Camera Actor
}

void AVRCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) {	 //This is a defualt function for Unreal
																						//HalfHeightAdjust and ScaledHalfHeightAdjust are parameters that unreal send automatically.
																						//Depends on the View and Height of the player.
	if (HalfHeightAdjust == 0.f) return;

	float StartBaseEyeHeight = BaseEyeHeight;											//BaseEyeHeight is property for Characters in Unreal Engine
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);						//With these we can make sure that the class is going to call the function 
	CrouchEyeOffset.Z += StartBaseEyeHeight - BaseEyeHeight - HalfHeightAdjust;			//Just modify the Vector  on Z
	Camera->SetRelativeLocation(FVector(BaseEyeHeight), false);				//Set the new BaseEyeHeight to Camera Actor
}

void AVRCharacter::CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) {
	if (Camera) {																		//If Camera Exists
		Camera->GetCameraView(DeltaTime, OutResult);									//Set values accordin to delta time and Outresult to Camera Actor
		OutResult.Location += CrouchEyeOffset;											//Set CrouchEyeOffset to Location of OutResult
																						//Note: OutResult is a pointer
	}

}

//This function make the CameraView more real
void AVRCharacter::BestCameraView() {
	FVector CurrentCameraActorLocation = Camera->GetComponentLocation();	//Se obtiene la posición de la cámara del jugador
	FVector CurrentLocation = GetActorLocation();							//Se obtiene la posición del jugador


	FVector NewCameraOffset = CurrentCameraActorLocation - CurrentLocation;	//Camera Offset = Camera location - character location			
	NewCameraOffset.Z = 0;													//Reset offset amount in camera z Axis to 0				
	//The next two lines pair the location of VRCamera and Camera character, both cameras are different.
	AddActorWorldOffset(NewCameraOffset);
	VRRoot->AddWorldOffset(-NewCameraOffset); //Vroot != Camera
}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &AVRCharacter::Jumping);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &AVRCharacter::StopJump);
	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &AVRCharacter::CrouchControl);
	PlayerInputComponent->BindAction(TEXT("GripLeft"), IE_Pressed, this, &AVRCharacter::GripLeft);
	PlayerInputComponent->BindAction(TEXT("GripRight"), IE_Pressed, this, &AVRCharacter::GripRight);
	PlayerInputComponent->BindAction(TEXT("GripLeft"), IE_Released, this, &AVRCharacter::ReleaseLeft);
	PlayerInputComponent->BindAction(TEXT("GripRight"), IE_Released, this, &AVRCharacter::ReleaseRight);

	PlayerInputComponent->BindAxis(TEXT("Forward"), this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("Right"), this, &AVRCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("ForwardL"), this, &AVRCharacter::Running);
}
