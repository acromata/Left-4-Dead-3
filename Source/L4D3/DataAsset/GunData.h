// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GunData.generated.h"

UENUM(BlueprintType)
enum EWeaponType : uint8
{
	Primary,
	Secondary
};
/**
 * 
 */
UCLASS()
class L4D3_API UGunData : public UDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UStaticMesh* Mesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Damage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 BulletCapacity;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsAutomatic;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TimeBetweenShots;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BulletRange;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<EWeaponType> WeaponType;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundBase* GunSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 AmmoInMag;

};
