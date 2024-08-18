// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/PawnSensingComponent.h"
#include "AIController.h"
#include "L4D3/Player/PlayerCharacter.h"
#include "ZombieAI.generated.h"

UENUM(BlueprintType)
enum EEnemyState : int8
{
	EIdleState,
	EChaseState
};

UCLASS()
class L4D3_API AZombieAI : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UPawnSensingComponent* PawnSensing;

public:
	// Sets default values for this character's properties
	AZombieAI();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:

	// Mesh
	TArray<USkeletalMesh*> ZombieMeshes;

	// Pawn Sensing
	UFUNCTION()
	void OnSeePawn(APawn* Pawn);

	UPROPERTY(BlueprintReadOnly)
	bool bCanSeePlayer;
	UPROPERTY(BlueprintReadOnly)
	float DistanceFromTarget;
	UPROPERTY(BlueprintReadOnly)
	FVector StartLocation;

	AAIController* AIController;

	// States
	void UpdateState(float DeltaTime);
	void SetState(EEnemyState NewState) { ActiveState = NewState; }
	EEnemyState ActiveState;

	void IdleState(float DeltaTime);
	void ChaseState(float DeltaTime);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chasing")
	float ChaseDistance;

	// Attacking
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attacking")
	float AttackDamage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attacking")
	float AttackingDistance;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attacking")
	UAnimMontage* AttackAnimation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attacking")
	float TimeBetweenAttacks;
	UPROPERTY(BlueprintReadOnly)
	float TimeSinceLastAttack;
	UPROPERTY(BlueprintReadOnly)
	bool bIsAttacking;
	UPROPERTY(BlueprintReadOnly)
	bool bHasDamagedPlayer;

	// Target
	APlayerCharacter* Target;

	// Health
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	int32 MaxHealth;
	UPROPERTY(BlueprintReadWrite)
	int32 CurrentHealth;

	// Sound
	void PlayRandomGrowl();

	UPROPERTY(EditAnywhere, Category = "Sound")
	TArray<USoundBase*> GrowlSounds;
	UPROPERTY(EditAnywhere, Category = "Sound")
	int32 ChanceToPlaySound;

public:

	void Damage(int32 Damage);
};
