// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalChecker.h"

// Sets default values
APortalChecker::APortalChecker()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	m_Distance = 140.f;
	m_DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	m_UpCheck = CreateDefaultSubobject<USceneComponent>(TEXT("UpCheck"));
	m_DownCheck = CreateDefaultSubobject<USceneComponent>(TEXT("DownCheck"));
	m_LeftCheck = CreateDefaultSubobject<USceneComponent>(TEXT("LeftCheck"));
	m_RightCheck = CreateDefaultSubobject<USceneComponent>(TEXT("RightCheck"));

	SetRootComponent(m_DefaultSceneRoot);
	m_UpCheck->SetupAttachment(RootComponent);
	m_DownCheck->SetupAttachment(RootComponent);
	m_LeftCheck->SetupAttachment(RootComponent);
	m_RightCheck->SetupAttachment(RootComponent);
	m_UpCheck->SetWorldLocation(FVector(0.f, 0.f, 140.f));
	m_DownCheck->SetWorldLocation(FVector(0.f, 0.f, -140.f));
	m_LeftCheck->SetWorldLocation(FVector(0.f, 140.f, 0.f));
	m_RightCheck->SetWorldLocation(FVector(0.f, -140.f, 0.f));


	/*
	m_Test1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("m_Test1"));
	m_Test1->SetupAttachment(m_UpCheck);
	m_Test2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("m_Test2"));
	m_Test2->SetupAttachment(m_DownCheck);
	m_Test3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("m_Test3"));
	m_Test3->SetupAttachment(m_LeftCheck);
	m_Test4 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("m_Test4"));
	m_Test4->SetupAttachment(m_RightCheck);

	static ConstructorHelpers::FObjectFinder<UStaticMesh>	Mesh(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	if (Mesh.Succeeded())
	{
		m_Test1->SetStaticMesh(Mesh.Object);
		m_Test2->SetStaticMesh(Mesh.Object);
		m_Test3->SetStaticMesh(Mesh.Object);
		m_Test4->SetStaticMesh(Mesh.Object);
	}
	m_Test1->SetRelativeScale3D(FVector(0.2f, 0.2f, 0.2f));
	m_Test2->SetRelativeScale3D(FVector(0.2f, 0.2f, 0.2f));
	m_Test3->SetRelativeScale3D(FVector(0.2f, 0.2f, 0.2f));
	m_Test4->SetRelativeScale3D(FVector(0.2f, 0.2f, 0.2f));
	*/
}

// Called when the game starts or when spawned
void APortalChecker::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APortalChecker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

