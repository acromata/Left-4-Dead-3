// Fill out your copyright notice in the Description page of Project Settings.


#include "L4D3/Player/PlayerCharacter.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"


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
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>("Weapon Mesh");
	WeaponMesh->SetupAttachment(Camera);

	// Speed
	SprintSpeed = 600.f;
	WalkSpeed = 350.f;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;

	// Shooting
	bCanShoot = true;
	TotalAmmo = 120;

	// Health
	MaxHealth = 100;
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Set equipped weapon
	if (IsValid(PrimaryWeapon))
	{
		EquippedWeapon = PrimaryWeapon;
		WeaponMesh->SetStaticMesh(EquippedWeapon->Mesh);
	}
	else if (IsValid(SecondaryWeapon))
	{
		EquippedWeapon = SecondaryWeapon;
		WeaponMesh->SetStaticMesh(EquippedWeapon->Mesh);
	}

	if (IsValid(EquippedWeapon))
	{
		EquippedWeapon->AmmoInMag = EquippedWeapon->BulletCapacity;
	}

	// Health
	CurrentHealth = MaxHealth;
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
	if (IsValid(EquippedWeapon))
	{
		if (EquippedWeapon->AmmoInMag > 0 && bCanShoot && !bIsReloading)
		{
			// Line Trace Setup
			FVector StartLocation = WeaponMesh->GetSocketLocation("FireLocation");
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
	}
}

void APlayerCharacter::CallReload()
{
	if (!bIsReloading)
	{
		FTimerHandle ReloadTimer;
		GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &APlayerCharacter::Reload, 1);
		bIsReloading = true;
	}

}

void APlayerCharacter::Reload()
{
	// Takes out ammo
	TotalAmmo += EquippedWeapon->AmmoInMag;
	EquippedWeapon->AmmoInMag = 0;

	// Adds Ammo
	EquippedWeapon->AmmoInMag = FMath::Clamp(EquippedWeapon->AmmoInMag + EquippedWeapon->BulletCapacity, 0, TotalAmmo);
	TotalAmmo = FMath::Clamp(TotalAmmo - EquippedWeapon->BulletCapacity, 0, 990);

	bIsReloading = false;
}

void APlayerCharacter::Interact()
{
	// Pickup gun
	if (IsValid(GunInRange))
	{
		UGunData* Gun = GunInRange->GunData;
		if (Gun->WeaponType == EWeaponType::Primary)
		{
			PrimaryWeapon = Gun;
		}
		else
		{
			SecondaryWeapon = Gun;
		}

		// Set mesh
		EquippedWeapon = Gun;
		WeaponMesh->SetStaticMesh(Gun->Mesh);

		// Destroy pickup
		GunInRange->Destroy();
	}
}
