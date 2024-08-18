// Fill out your copyright notice in the Description page of Project Settings.


#include "L4D3/Enemy/ZombieAI.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values
AZombieAI::AZombieAI()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Capsule
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	// Pawn sensing
	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>("PawnSensing");
	PawnSensing->SetPeripheralVisionAngle(70.f);

	// Attacking
	AttackDamage = 30.f;
	TimeBetweenAttacks = 0.8f;
	AttackingDistance = 100.f;

	// Chasing
	ChaseDistance = 2000.f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;

	// Health
	MaxHealth = 100;

	// Sound
	ChanceToPlaySound = 5;
}

// Called when the game starts or when spawned
void AZombieAI::BeginPlay()
{
	Super::BeginPlay();
	
	// Set AI Controller
	AIController = Cast<AAIController>(Controller);

	// State default
	ActiveState = EEnemyState::EIdleState;

	// Set player as target
	Target = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

	// Pawn Sensing Bindings
	PawnSensing->OnSeePawn.AddDynamic(this, &AZombieAI::OnSeePawn);

	// Start Location
	StartLocation = GetActorLocation();

	// Health
	CurrentHealth = MaxHealth;

	// Set mesh
	int32 RandNum = FMath::RandRange(0, ZombieMeshes.Num() - 1);
	if (ZombieMeshes.IsValidIndex(RandNum))
	{
		GetMesh()->SetSkeletalMesh(ZombieMeshes[RandNum]);
	}
	
}

// Called every frame
void AZombieAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update state
	UpdateState(DeltaTime);

	// Update distance between target and self
	DistanceFromTarget = FVector::Distance(Target->GetActorLocation(), GetActorLocation());

	// Update time since last attack
	TimeSinceLastAttack += DeltaTime;
}

void AZombieAI::OnSeePawn(APawn* Pawn)
{
	if (Pawn == UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		bCanSeePlayer = true;
	}
}

void AZombieAI::UpdateState(float DeltaTime)
{
	switch (ActiveState)
	{
	case EIdleState:
		IdleState(DeltaTime);
		break;
	case EChaseState:
		ChaseState(DeltaTime);
		break;
	default:
		SetState(EEnemyState::EIdleState);
		break;
	}

	// Print state
	GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Green, FString::Printf(TEXT("Active State: %s"),
		*UEnum::GetValueAsString<EEnemyState>(ActiveState)));
}

void AZombieAI::IdleState(float DeltaTime)
{
	if (bCanSeePlayer)
	{
		// Chase player
		SetState(EEnemyState::EChaseState);
	}
	else
	{
		// Player too far, return to start location
		if (IsValid(AIController) && GetActorLocation() != StartLocation)
		{
			AIController->MoveToLocation(StartLocation);
		}
	}
}

void AZombieAI::ChaseState(float DeltaTime)
{
	if (DistanceFromTarget <= AttackingDistance)
	{
		// Stop movement
		AIController->StopMovement();

		if (TimeSinceLastAttack >= TimeBetweenAttacks)
		{
			// Attack
			Target->Damage(AttackDamage);

			PlayRandomGrowl();

			// Reset timer
			TimeSinceLastAttack = 0;
		}
	}
	else if(DistanceFromTarget <= ChaseDistance)
	{
		// Chase
		if (IsValid(AIController) && !AIController->IsFollowingAPath())
		{
			AIController->MoveToActor(Target);
			PlayRandomGrowl();
		}
	}
	else
	{
		bCanSeePlayer = false;
		SetState(EEnemyState::EIdleState);
	}
}

void AZombieAI::Damage(int32 Damage)
{
	// Subtract health
	CurrentHealth = FMath::Clamp(CurrentHealth -= Damage, 0, MaxHealth);

	SetState(EEnemyState::EChaseState);

	PlayRandomGrowl();

	// Die
	if (CurrentHealth <= 0)
	{
		// Die
		Destroy();
	}
}

void AZombieAI::PlayRandomGrowl()
{
	if (FMath::RandRange(0, ChanceToPlaySound) == 0)
	{
		int32 RandNum = FMath::RandRange(0, GrowlSounds.Num() - 1);
		if (GrowlSounds.IsValidIndex(RandNum))
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), GrowlSounds[RandNum], GetActorLocation());
		}
	}
}


