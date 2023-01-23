// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MotionControllerComponent.h"
#include "VRController.generated.h"



UCLASS()
class VR_ENGLISH_API AVRController : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVRController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//VISTA
	void ActiveVibration();

	//MODELO

public:
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UStaticMeshComponent> AVRPickupMeshComponent;
	bool bIsClimbing = false;
	bool bIsPickingup = false;
	bool bIsScaling = false;
	FVector ObjectLocation = FVector(0.f);


	//CONTROLADOR
public:

	void SetHand(EControllerHand Hand) { MotionController->SetTrackingSource(Hand); }
	void PairController(AVRController* Controller);

	//Helpers
	int ObjectSelect();

	//Default Suboject
	UPROPERTY(VisibleAnywhere)
		UMotionControllerComponent* MotionController;

	//Parameters
	UPROPERTY(EditDefaultsOnly)
		class UHapticFeedbackEffect_Base* HapticEffect;

	AVRController* OtherController;
	FVector ClimbingStartLocation = FVector(0, 0, 0);;
	FVector ScalingStartLocation = FVector(0, 0, 0);;

	bool GripClimb();
	bool ReleaseClimb();
	bool GripPickup();
	bool GripScaleClimb();
	bool ReleaseScale(double LocationLeftC, double LocationRightC, double PlayerHeadLocation);

	//state
	bool bCanClimb = false;
	bool bCanPickup = false;
	bool bCanScale = false;
	bool flagScale = false;
	bool flagRelease = false;

private:
	//Callbacks
	UFUNCTION()
		void ActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);
	UFUNCTION()
		void ActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UPROPERTY(VisibleAnywhere)
		USceneComponent* SceneComponent;
};