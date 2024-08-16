// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../DataAsset/ItemData.h"
#include "HealthItemData.generated.h"

/**
 * 
 */
UCLASS()
class L4D3_API UHealthItemData : public UItemData
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 HealthToHeal;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsTemporary;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpeedMultiplier;

};
