// Fill out your copyright notice in the Description page of Project Settings.


#include "L4D3/Pickup/WeaponPickup.h"
#include "Components/SphereComponent.h"
#include "../Player/PlayerCharacter.h"

// Sets default values
AWeaponPickup::AWeaponPickup()
{
	SphereCollision = CreateDefaultSubobject<USphereComponent>("Collision");
	SphereCollision->SetupAttachment(RootComponent);
	SphereCollision->SetSphereRadius(60.f);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	Mesh->SetupAttachment(SphereCollision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
}

// Called when the game starts or when spawned
void AWeaponPickup::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(Timer, this, &AWeaponPickup::BeginPlayDelay, .5f);
}

void AWeaponPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (IsValid(Player))
	{
		Player->ItemInRange = this;
	}
}

void AWeaponPickup::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (IsValid(Player))
	{
		Player->ItemInRange = nullptr;
	}
}

void AWeaponPickup::BeginPlayDelay()
{
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeaponPickup::OnOverlapBegin);
	SphereCollision->OnComponentEndOverlap.AddDynamic(this, &AWeaponPickup::OnOverlapEnd);

	if (IsValid(ItemData))
	{
		Mesh->SetStaticMesh(ItemData->Mesh);
	}
}

