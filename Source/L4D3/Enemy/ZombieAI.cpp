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

	// Zombie
	RadiusToAlert = 500.f;
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
	float RandHealth = FMath::RandRange(1, 13);
	RandHealth /= 10;
	CurrentHealth = MaxHealth * RandHealth;

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
	//GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Green, FString::Printf(TEXT("Active State: %s"),
	//	*UEnum::GetValueAsString<EEnemyState>(ActiveState)));
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

			// Play sound
			PlayRandomGrowl();

			// Reset timer
			TimeSinceLastAttack = 0;

			// Play anim
			UAnimInstance* AnimationInstance = GetMesh()->GetAnimInstance();
			if (IsValid(AnimationInstance) && IsValid(AttackAnimation))
			{
				AnimationInstance->Montage_Play(AttackAnimation);
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, "Attacked");
			}

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

	// Alert nearby zombies
	if (ActiveState != EEnemyState::EChaseState)
	{
		// create tarray for hit results
		TArray<FHitResult> OutHits;

		// create a collision sphere
		FCollisionShape ColSphere = FCollisionShape::MakeSphere(RadiusToAlert);

		// draw collision sphere
		DrawDebugSphere(GetWorld(), GetActorLocation(), ColSphere.GetSphereRadius(), 50, FColor::Purple, false, 2.f);

		// check if something got hit in the sweep
		bool bIsHit = GetWorld()->SweepMultiByChannel(OutHits, GetActorLocation(), GetActorLocation(), FQuat::Identity, ECC_WorldStatic, ColSphere);

		if (bIsHit)
		{
			// loop through TArray
			for (auto& Hit : OutHits)
			{
				AZombieAI* ZombieHit = Cast<AZombieAI>(Hit.GetActor());
				if(IsValid(ZombieHit))
				{
					ZombieHit->SetState(EEnemyState::EChaseState);
				}
			}
		}
	}

	// Chase player on hit
	SetState(EEnemyState::EChaseState);

	// Die if no health
	if (CurrentHealth <= 0)
	{
		// Play sound
		PlayRandomGrowl(true);

		// Play anim
		int8 RandNum = FMath::RandRange(0, DeathAnimations.Num() - 1);
		if (DeathAnimations.IsValidIndex(RandNum))
		{
			GetMesh()->PlayAnimation(DeathAnimations[RandNum], false);
		}

		// Disable collision
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		bIsDead = true;

	}
	else
	{
		// Play sound
		PlayRandomGrowl();
	}
}

void AZombieAI::PlayRandomGrowl(bool IsGuaranteed)
{
	if ((FMath::RandRange(0, ChanceToPlaySound) == 0 || IsGuaranteed) && !bIsDead)
	{
		int32 RandNum = FMath::RandRange(0, GrowlSounds.Num() - 1);
		if (GrowlSounds.IsValidIndex(RandNum))
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), GrowlSounds[RandNum], GetActorLocation());
		}
	}
}


