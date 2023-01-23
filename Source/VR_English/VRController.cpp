// Fill out your copyright notice in the Description page of Project Settings.


#include "VRController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
//#include "VRCharacter.h"


// Sets default values
AVRController::AVRController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController"));
	SetRootComponent(MotionController);

}

// Called when the game starts or when spawned
void AVRController::BeginPlay()
{
	Super::BeginPlay();

	OnActorBeginOverlap.AddDynamic(this, &AVRController::ActorBeginOverlap);
	OnActorEndOverlap.AddDynamic(this, &AVRController::ActorBeginOverlap);
}

// Called every frame
void AVRController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector HandContollerDelta = FVector(0, 0, 0);

	if (bIsScaling) {															//If player is Scaling
		HandContollerDelta = GetActorLocation() - ScalingStartLocation;			//Get the real location of Scaling hand
		GetAttachParentActor()->AddActorWorldOffset(-HandContollerDelta);		//Then we attach that result to Actor
	}

	if (bIsClimbing) {															//If player is Climbing
		HandContollerDelta = GetActorLocation() - ClimbingStartLocation;		//Get the real location of Climbing hand
		GetAttachParentActor()->AddActorWorldOffset(-HandContollerDelta);		//Then we attach that result to Actor
	}


}


/*---------------------------------------------------------------VISTA------------------------------------------------------------------------------------------------*/
//This function allows to activate the Vibration in controllers
void AVRController::ActiveVibration() {
	APawn* Pawn = Cast<APawn>(GetAttachParentActor());
	if (Pawn != nullptr) {
		APlayerController* Controller = Cast<APlayerController>(Pawn->GetController());
		if (Controller != nullptr) {
			Controller->PlayHapticEffect(HapticEffect, MotionController->GetTrackingSource());		//This command line activates the Vibration in controller
		}
	}
}

/*---------------------------------------------------------------MODELO------------------------------------------------------------------------------------------------*/



/*---------------------------------------------------------------CONTROLADOR------------------------------------------------------------------------------------------------*/

//This an Unreal function, this function is activate when an event starts
void AVRController::ActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor) {				//When and event start
																									//In this case when a control touch an object
	int Selector = ObjectSelect();																	//Take the value of the function
	switch (Selector)
	{
	case 0:
		bCanClimb = true;																			//Allows Actor to Climb
		ActiveVibration();																			//Active vibration in Oculus controllers
		break;
	case 1:
		bCanScale = true;																			//Allows Actor to Scale
		ActiveVibration();																			//Active vibration in Oculus controllers
		break;

	case 2:
		bCanPickup = true;
		break;
	default:
		break;
	}



}

//This an Unreal function, this function is activate when an event ends
void AVRController::ActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor) {				///When Event finish
																								//Set all Values in false
	bCanPickup = false;
	bCanClimb = false;
	bCanScale = false;
}


//This function select the object by Tag
int AVRController::ObjectSelect() {
	TArray<AActor*> OverlappingActors;															//Whe create an actor array
	TArray<UActorComponent*> Comps = GetComponentsByTag(UBoxComponent::StaticClass(), TEXT("BoxScalable"));
	GetOverlappingActors(OverlappingActors);													//We get the actors that player is touching
	for (AActor* OverlappingActor : OverlappingActors) {
		if (OverlappingActor->ActorHasTag(TEXT("Climbable"))) {									//Detect if the object has a tag 
			return 0;																			//0 returns to be used on ActorBeginOverlap
		}

		if (OverlappingActor->ActorHasTag(TEXT("Scalable"))) {									//Detect if the object has a tag 
			ObjectLocation = OverlappingActor->GetActorLocation();								//1 returns to be used on ActorBeginOverlap
			return 1;
		}

		if (OverlappingActor->ActorHasTag(TEXT("Pickupbable"))) {
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Pickupbable"));
			return 2;
		}


	}

	return 20;																					//Returns 20 if function doesnt detect a tag
}


//This function allows to climb
bool AVRController::GripClimb() {

	if (!bCanClimb) return 0;																//bCanClimb has to be in true 

	if (!bIsClimbing) {

		bIsClimbing = true;
		ClimbingStartLocation = GetActorLocation();											//Get the actual location of the Oculus Control
		OtherController->bIsClimbing = false;												//Set the other controller in false

		ACharacter* Character = Cast<ACharacter>(GetAttachParentActor());					//We need to cast the character to be use un Controller.cpp
		if (Character != nullptr) {
			Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);	//Set the character im Move flying
			return 1;																		//Returns 1 if player is Climbing
		}

	}
	return 0;																				//Returns 0 if player is not climbing
}

//This function allows to scale
bool AVRController::GripScaleClimb() {
	if (!bCanScale) return 0;																//bCanScale has to be in true 

	if (!bIsScaling) {

		bIsScaling = true;
		ScalingStartLocation = GetActorLocation();											//Get the actual location of the Oculus Control
		OtherController->bIsScaling = false;												//Set the other controller in false

		ACharacter* Character = Cast<ACharacter>(GetAttachParentActor());					//We need to cast the character to be use un Controller.cpp
		if (Character != nullptr) {
			//Character->AttachToComponent();
			Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);	//Set the character im Move flying
			return 1;																		//Returns 1 if player is Climbing
		}
	}
	return 0;
}




//-----------------------IGNORE THIS FUNCTION------------------------------------//
bool AVRController::GripPickup() {
	if (!bCanPickup) return 0;

	if (!bIsPickingup) {

		bIsPickingup = true;
		OtherController->bIsPickingup = false;

		//ACharacter* Character = Cast<ACharacter>(GetAttachParentActor());
		AActor* Actor = Cast<AActor>(GetAttachParentActor());
		if (Actor != nullptr) {
			//Actor->AttachToComponent(PickupAccess->PickupStaticMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, NAME_None);
			return 1;
		}
	}
	return 0;
}



//This function constrolls when character wants to stop climb
bool AVRController::ReleaseClimb() {
	if (bIsClimbing) {																								//If bIsClimbing = true
		bIsClimbing = false;																						//Set bIsClimbing == false
		ACharacter* Character = Cast<ACharacter>(GetAttachParentActor());											//We Cast the character to use it in Controller.cpp
		if (Character != nullptr) {
			Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);						//Set the Character in falling
			return 0;																								//return 0 if character stops climbing
		}
	}
	return 1;																										//Returns 1 if character is still climbing
}


//This function constrolls when character wants to scale an object
bool AVRController::ReleaseScale(double LocationLeftC, double LocationRightC, double PlayerHeadLocation) {			//this function recive 3 values

	if (bIsScaling) {																								//If bIsScaling = true
		bIsScaling = false;																							//Set bIsScaling == false
		ACharacter* Character = Cast<ACharacter>(GetAttachParentActor());											//We Cast the character to use it in Controller.cpp
		if ((LocationLeftC < PlayerHeadLocation * .75) && (LocationRightC < PlayerHeadLocation * .75)) {				//Check if ControllLeft and ControllRight are less than 75% PlayerHeadLocation 
			if (Character != nullptr) {

				Character->SetActorLocation(ObjectLocation + FVector(50, 0, 150));									//Set the character above the Scalable object 	
				Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);					//Set the Character in falling
				OtherController->bIsScaling = false;																//Set the other controll in false
				bIsScaling = false;																					//Set bIsScaling in false
				return 0;																							//returns 0 if character scale the object
			}
		}
		else {
			Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);						//Set the Character in falling						
			return 0;																								//returns 0 if character stops scaling
		}
	}
	return 1;																										//Returns 0 if character is still climbing								

}


//This function pair the Oculus controllers according with Motion Controller component
void AVRController::PairController(AVRController* Controller) {
	OtherController = Controller;
	OtherController->OtherController = this;

}