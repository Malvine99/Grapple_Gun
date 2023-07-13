// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GrappleGun.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UCharacterMovementComponent;
class UCableComponent;

UCLASS()
class COURSEWORK4_API AGrappleGun : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGrappleGun();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
		UStaticMeshComponent* StaticMeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
		UCharacterMovementComponent* MoveComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rope")
		UCableComponent* CableComp;

protected:
	//movement functions
	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Value);
	void LookUpAtRate(float Value);

	//action functions
	void FireGrapple();
	void ReelIn(float dt);
	void BreakGrapple();
	void MoveRope(float dt);
	void Swing(float dt);
	void Pull();

	//state functions
	void StartReelIn();
	void StopReelIn();
	void StartPull();

	void CatchUpRope(float dt);

	//bools
	bool bReelIn;
	bool bHitWall;
	bool bAttached;
	bool bPull;

	FVector hookLocation;
	FVector RopeRelativeLocation;
	float trackTime;
	bool bDelay;

	//editable variables
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		float BaseTurnRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		float BaseLookUpAtRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
		float grappleRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
		float grappleSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
		float swingSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
		float pullForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope")
		float defaultRopeLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope")
		float ropeSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope")
		float minDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope")
		float ropeOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope")
		float ropeDelay;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
