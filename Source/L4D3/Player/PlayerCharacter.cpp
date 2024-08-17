// Fill out your copyright notice in the Description page of Project Settings.


#include "L4D3/Player/PlayerCharacter.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Camera
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(RootComponent);
	Camera->bUsePawnControlRotation = true;

	// Weapon
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>("Item Mesh");
	ItemMesh->SetupAttachment(Camera);

	// Speed
	SprintSpeed = 600.f;
	WalkSpeed = 350.f;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;

	// Shooting
	bCanShoot = true;
	TotalAmmo = 120;

	// Health
	MaxHealth = 100;
	TemporaryHealthDecayRate = 3.f;
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Health
	CurrentHealth = MaxHealth;

	FTimerHandle TempHealthTimer;
	GetWorld()->GetTimerManager().SetTimer(TempHealthTimer, this, &APlayerCharacter::SubtractTempHealth, TemporaryHealthDecayRate, true);
}
	

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Add input mapping context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMapping, 0);
		}
	}

	// Input actions
	if (UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		Input->BindAction(JumpAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Jump);

		Input->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &APlayerCharacter::StartCrouch);
		Input->BindAction(CrouchAction, ETriggerEvent::Completed, this, &APlayerCharacter::EndCrouch);

		Input->BindAction(FireAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Fire);
		Input->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &APlayerCharacter::CallReload);

		Input->BindAction(InteractAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Interact);
		Input->BindAction(DropItemAction, ETriggerEvent::Triggered, this, &APlayerCharacter::DropEquippedItem);

	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	// Input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (IsValid(Controller))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (IsValid(Controller))
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APlayerCharacter::Jump()
{
	ACharacter::Jump();
}

void APlayerCharacter::StartCrouch()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void APlayerCharacter::EndCrouch()
{
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void APlayerCharacter::Fire()
{
	if (UGunData* EquippedWeapon = Cast<UGunData>(EquippedItem))
	{
		if (EquippedWeapon->AmmoInMag > 0 && bCanShoot && !bIsReloading)
		{
			Shoot(EquippedWeapon);
		}
	}
	else if(UHealthItemData* EquippedHealth = Cast<UHealthItemData>(EquippedItem))
	{
		Heal(EquippedHealth->HealthToHeal, EquippedHealth->bIsTemporary);
	}
}

void APlayerCharacter::Shoot(UGunData* EquippedWeapon)
{
	// Line Trace Setup
	FVector StartLocation = ItemMesh->GetSocketLocation("FireLocation");
	FVector EndLocation = StartLocation + (Camera->GetComponentRotation().Vector() * EquippedWeapon->BulletRange);
	FHitResult HitResult;
	FCollisionQueryParams ColParams;
	ColParams.AddIgnoredActor(this);

	// Line Trace
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, ColParams);
	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (IsValid(HitActor))
		{
			HitActor->Destroy();
		}
	}

	// Debug
	DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 1.f);

	// Subtract ammo
	EquippedWeapon->AmmoInMag--;

	// Play gun sound
	UGameplayStatics::PlaySound2D(GetWorld(), EquippedWeapon->GunSound);

	// Time between shots
	bCanShoot = false;
	FTimerHandle TBSTimer;
	GetWorld()->GetTimerManager().SetTimer(TBSTimer, this, &APlayerCharacter::EnableShooting, EquippedWeapon->TimeBetweenShots);
}

void APlayerCharacter::CallReload()
{
	UGunData* EquippedWeapon = Cast<UGunData>(EquippedItem);
	if (!bIsReloading && IsValid(EquippedWeapon))
	{
		FTimerHandle ReloadTimer;
		GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &APlayerCharacter::Reload, 1);
		bIsReloading = true;
	}
}

void APlayerCharacter::Reload()
{
	UGunData* EquippedWeapon = Cast<UGunData>(EquippedItem);
	if (IsValid(EquippedWeapon))
	{
		// Takes out ammo
		TotalAmmo += EquippedWeapon->AmmoInMag;
		EquippedWeapon->AmmoInMag = 0;

		// Adds Ammo
		EquippedWeapon->AmmoInMag = FMath::Clamp(EquippedWeapon->AmmoInMag + EquippedWeapon->BulletCapacity, 0, TotalAmmo);
		TotalAmmo = FMath::Clamp(TotalAmmo - EquippedWeapon->BulletCapacity, 0, 990);

		bIsReloading = false;
	}
}

void APlayerCharacter::Interact()
{
	// Pickup gun
	if (IsValid(ItemInRange))
	{
		// If gun
		UItemData* Item = ItemInRange->ItemData;
		if (UGunData* Gun = Cast<UGunData>(Item))
		{
			if (Gun->ItemType == EItemType::Primary)
			{
				DropItem(PrimaryWeapon);

				// Pickup weapon
				PrimaryWeapon = Gun;
			}
			else
			{
				DropItem(SecondaryWeapon);

				// Pickup weapon
				SecondaryWeapon = Gun;
			}
		}
		// If healing
		else if (UHealthItemData* Health = Cast<UHealthItemData>(Item))
		{
			if (Health->ItemType == EItemType::Primary)
			{
				DropItem(PrimaryHealingItem);

				// Pickup item
				PrimaryHealingItem = Health;
			}
			else
			{
				DropItem(SecondaryHealingItem);

				// Pickup item
				SecondaryHealingItem = Health;
			}		
		}

		// Set mesh
		EquippedItem = Item;
		ItemMesh->SetStaticMesh(Item->Mesh);

		// Destroy pickup
		ItemInRange->Destroy();
	}
}

void APlayerCharacter::Damage(int32 Damage)
{
	// Check if player has temporary health
	if (TemporaryHealth <= 0)
	{
		// Subtract health
		CurrentHealth = FMath::Clamp(CurrentHealth -= Damage, 0, MaxHealth);

		// Die
		if (CurrentHealth <= 0)
		{
			// Die
		}
	}
	else
	{
		// Subtract temporary health
		TemporaryHealth = FMath::Clamp(TemporaryHealth -= Damage, 0, MaxHealth);
	}

}

void APlayerCharacter::Heal(int32 HealthToAdd, bool bIsTemporary)
{
	if (CurrentHealth < MaxHealth)
	{
		if (bIsTemporary)
		{
			if (CurrentHealth + TemporaryHealth < MaxHealth)
			{
				TemporaryHealth = FMath::Clamp(TemporaryHealth += HealthToAdd, 0, MaxHealth - CurrentHealth);
			}
		}
		else
		{
			CurrentHealth = FMath::Clamp(CurrentHealth += HealthToAdd, 0, MaxHealth);
		}

		if (EquippedItem->ItemType = EItemType::Primary)
		{
			PrimaryHealingItem = nullptr;
		}
		else
		{
			SecondaryHealingItem = nullptr;
		}

		EquippedItem = nullptr;
		ItemMesh->SetStaticMesh(nullptr);
	}

}

void APlayerCharacter::SubtractTempHealth()
{
	TemporaryHealth = FMath::Clamp(TemporaryHealth = TemporaryHealth - 1, 0, MaxHealth - CurrentHealth);
}

void APlayerCharacter::DropEquippedItem()
{
	DropItem(EquippedItem);
	EquippedItem = nullptr;
	ItemMesh->SetStaticMesh(nullptr);
}

void APlayerCharacter::DropItem(UItemData* Item)
{
	if (IsValid(Item))
	{
		AWeaponPickup* ItemDrop = GetWorld()->SpawnActor<AWeaponPickup>(GetActorLocation(), GetActorRotation());
		ItemDrop->ItemData = Item;
		ItemDrop->Mesh->SetStaticMesh(Item->Mesh);
		ItemDrop->SphereCollision->SetSimulatePhysics(true);

		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::White, (TEXT("Item Dropped: %s"), Item->GetName()));
	}
}
