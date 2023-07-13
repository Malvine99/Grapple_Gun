// Fill out your copyright notice in the Description page of Project Settings.


#include "GrappleGun.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CableComponent.h"
#include "Math/Vector.h"
#include "Math/UnrealMathUtility.h"

// Sets default values
AGrappleGun::AGrappleGun()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerMesh"));
	StaticMeshComp->SetupAttachment(RootComponent);

	MoveComp = GetCharacterMovement();

	CableComp = CreateDefaultSubobject<UCableComponent>(TEXT("CableComp"));
	CableComp->SetupAttachment(StaticMeshComp);

	BaseTurnRate = 45.f;
	BaseLookUpAtRate = 45.f;
	RopeRelativeLocation = StaticMeshComp->GetComponentLocation() - FVector(0, ropeOffset, 0);
	trackTime = 0.f;
	bDelay = false;
}

void AGrappleGun::MoveForward(float Value) {
	if ((Controller) && (Value != 0)) {
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		AddMovementInput(Direction, Value);
	}
}

void AGrappleGun::MoveRight(float Value) {
	if ((Controller) && (Value != 0)) {
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Direction, Value);
	}

}

void AGrappleGun::TurnAtRate(float Value) {
	AddControllerYawInput(Value * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AGrappleGun::LookUpAtRate(float Value) {
	AddControllerPitchInput(Value * BaseLookUpAtRate * GetWorld()->GetDeltaSeconds());
}

// Called every frame
void AGrappleGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDelay) {
		trackTime += DeltaTime;
		CatchUpRope(DeltaTime);

		if (trackTime > ropeDelay) {
			BreakGrapple();
			bDelay = false;
		}
	}
	else {
		if (bHitWall) {
			if (!bAttached) {
				MoveRope(DeltaTime);
			}
			else {
				if (bPull) {
					Pull();
					bDelay = true;
					trackTime = 0.f;
				}
				else {
					if (bReelIn) {
						ReelIn(DeltaTime);
					}
					else {
						Swing(DeltaTime);
					}
				}
			}
		}
	}

}

// Called to bind functionality to input
void AGrappleGun::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AGrappleGun::BreakGrapple);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AGrappleGun::FireGrapple);
	PlayerInputComponent->BindAction("Pull", IE_Pressed, this, &AGrappleGun::StartPull);
	PlayerInputComponent->BindAction("ReelIn", IE_Pressed, this, &AGrappleGun::StartReelIn);
	PlayerInputComponent->BindAction("ReelIn", IE_Released, this, &AGrappleGun::StopReelIn);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGrappleGun::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGrappleGun::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AGrappleGun::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AGrappleGun::LookUpAtRate);

}

//action functions
void  AGrappleGun::FireGrapple() {
		
	//line trace to nearest object, set hookLocation to grapple to

	FVector TraceEndLocation = RootComponent->GetComponentLocation() + (RootComponent->GetForwardVector() * grappleRange);
	FHitResult Hit;
	FCollisionObjectQueryParams ObjectQueryParams(ECC_WorldStatic);
	FCollisionQueryParams QueryParams(FName(), true, this);

	//const FName TraceTag("MyTraceTag");

	//GetWorld()->DebugDrawTraceTag = TraceTag;

	//QueryParams.TraceTag = TraceTag;

	bool Result = GetWorld()->LineTraceSingleByObjectType(Hit, RootComponent->GetComponentLocation(), TraceEndLocation, ObjectQueryParams, QueryParams);

	if (Result) {
		hookLocation = Hit.Location;
		bHitWall = true;
		bAttached = false;
	}
	else {
		BreakGrapple();
	}

}

void AGrappleGun::ReelIn(float dt) {
	//launch character towards hook location
	FVector Direction = hookLocation - GetActorLocation();
	float Speed = dt * grappleSpeed;
	FVector LaunchVelocity = Direction * Speed;

	LaunchCharacter(LaunchVelocity, true, true);

	CatchUpRope(dt);

}

void  AGrappleGun::BreakGrapple() {
	bHitWall = false;
	bAttached = false;
	//TODO
	//reset grapple line location and make invisible
	CableComp->SetVisibility(false);
	CableComp->SetWorldLocation(FVector(0, 0, 0));
	CableComp->EndLocation = FVector(0, 0, 0);
}

void  AGrappleGun::MoveRope(float dt) {

	CableComp->SetVisibility(true);
	
	FVector Direction = CableComp->GetComponentLocation() - hookLocation;
	float Distance = Direction.Size();

	if (Distance <= minDistance) {
		bAttached = true;
	}
	else {
		FVector newLoc = FMath::VInterpTo(CableComp->GetComponentLocation(), hookLocation, dt, ropeSpeed);
		CableComp->SetWorldLocation(newLoc);
	}
}

void  AGrappleGun::Swing(float dt) {
	FVector direction = GetActorLocation() - hookLocation;
	float dot = Dot3(GetVelocity(), direction);
	direction.Normalize();
	FVector result = dot * direction * -2 ;

	MoveComp->AddForce(result);
	CableComp->SetWorldLocation(hookLocation);
	CableComp->EndLocation = RopeRelativeLocation;
}

void  AGrappleGun::Pull() {

	FVector Direction = hookLocation - GetActorLocation();
	FVector LaunchVelocity = Direction * pullForce;

	MoveComp->AddImpulse(Direction, true);

	//BreakGrapple();
	bPull = false;

}

//state functions
void  AGrappleGun::StartReelIn() {
	bReelIn = true;
}

void  AGrappleGun::StopReelIn() {
	bReelIn = false;
}

void AGrappleGun::StartPull()
{
	FireGrapple();
	bPull = true;
}

void AGrappleGun::CatchUpRope(float dt)
{
	CableComp->SetWorldLocation(hookLocation);
	FVector newLoc = FMath::VInterpTo(CableComp->EndLocation, RopeRelativeLocation, dt, 250);
	CableComp->EndLocation = newLoc;
}
