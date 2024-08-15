// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../DataAsset/GunData.h"
#include "WeaponPickup.generated.h"

UCLASS()
class L4D3_API AWeaponPickup : public AActor
{
	GENERATED_BODY()

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class USphereComponent* SphereCollision;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* Mesh;
	
public:	
	// Sets default values for this actor's properties
	AWeaponPickup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:

	UPROPERTY(EditAnywhere, Category = "Item")
	UGunData* GunData;

};
