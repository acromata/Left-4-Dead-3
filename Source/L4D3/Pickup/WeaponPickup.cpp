// Fill out your copyright notice in the Description page of Project Settings.


#include "L4D3/Pickup/WeaponPickup.h"
#include "Components/SphereComponent.h"
#include "../Player/PlayerCharacter.h"

// Sets default values
AWeaponPickup::AWeaponPickup()
{
	SphereCollision = CreateDefaultSubobject<USphereComponent>("Collision");
	SphereCollision->SetupAttachment(RootComponent);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	Mesh->SetupAttachment(SphereCollision);
}

// Called when the game starts or when spawned
void AWeaponPickup::BeginPlay()
{
	Super::BeginPlay();
	
	Mesh->SetStaticMesh(GunData->Mesh);

	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeaponPickup::OnOverlapBegin);
	SphereCollision->OnComponentEndOverlap.AddDynamic(this, &AWeaponPickup::OnOverlapEnd);

}

void AWeaponPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (IsValid(Player))
	{
		Player->GunInRange = this;
	}
}

void AWeaponPickup::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (IsValid(Player))
	{
		Player->GunInRange = nullptr;
	}
}

