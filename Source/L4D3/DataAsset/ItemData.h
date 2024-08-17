// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemData.generated.h"

UENUM(BlueprintType)
enum EItemType : uint8
{
	Primary,
	Secondary
};

UCLASS()
class L4D3_API UItemData : public UDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UStaticMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<EItemType> ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* ItemIcon;
};
