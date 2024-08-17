// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../DataAsset/ItemData.h"
#include "GunData.generated.h"


/**
 * 
 */
UCLASS()
class L4D3_API UGunData : public UItemData
{
	GENERATED_BODY()
	
public:

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
	USoundBase* GunSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 AmmoInMag;

};
