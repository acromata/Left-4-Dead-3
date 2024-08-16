// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "L4D3/DataAsset/GunData.h"
#include "L4D3/DataAsset/HealthItemData.h"
#include "L4D3/Pickup/WeaponPickup.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class L4D3_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* ItemMesh;

protected:

	// Input context
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	class UInputMappingContext* InputMapping;


	// Input actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	class UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	class UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	class UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	class UInputAction* CrouchAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	class UInputAction* FireAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	class UInputAction* ReloadAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	class UInputAction* InteractAction;


public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	// Movement
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Jump();

	// Slow Walk
	void StartCrouch();
	void EndCrouch();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MoveSpeed")
	float SprintSpeed;
	float WalkSpeed;

	// Use Item
	void Fire();

	UPROPERTY(BlueprintReadWrite)
	UItemData* EquippedItem;
	
	// Weapons
	void Shoot(UGunData* EquippedWeapon);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items")
	UGunData* PrimaryWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items")
	UGunData* SecondaryWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalAmmo;
	UPROPERTY(BlueprintReadWrite)
	int32 AmmoInMag;
	UPROPERTY(BlueprintReadOnly)
	bool bCanShoot;
	UPROPERTY(BlueprintReadOnly)
	bool bIsReloading;

	void CallReload();
	UFUNCTION(BlueprintCallable)
	void Reload();
	void EnableShooting() { bCanShoot = true; }

	// Healing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items")
	UHealthItemData* HealingItem;

	// Health
	UFUNCTION(BlueprintCallable)
	void Damage(int32 Damage);
	UFUNCTION(BlueprintCallable)
	void Heal(int32 HealthToAdd, bool bIsTemporary = false);

	void SubtractTempHealth();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	int32 MaxHealth;
	UPROPERTY(BlueprintReadWrite)
	int32 CurrentHealth;
	UPROPERTY(BlueprintReadOnly)
	int32 TemporaryHealth;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float TemporaryHealthDecayRate;

	// When below 40, slow down movement

	// Interact
	void Interact();

public:

	UPROPERTY(BlueprintReadOnly)
	AWeaponPickup* ItemInRange;
};
