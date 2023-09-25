// Fill out your copyright notice in the Description page of Project Settings.


#include "BodyClone.h"
#include "PP3Character.h"
// Sets default values
ABodyClone::ABodyClone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	m_DefaultSceneRoot = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DefaultSceneRoot"));
	m_Arms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Arms"));
	m_ArmsGun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmsGun"));
	m_Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	m_Body = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Body"));
	m_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Gun"));
	m_Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	m_Grabsocket = CreateDefaultSubobject<USceneComponent>(TEXT("Grabsocket"));
	m_Sphere1 = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere1"));
	m_Sphere2 = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere2"));
	SetRootComponent(m_DefaultSceneRoot);
	m_Capsule->SetupAttachment(m_DefaultSceneRoot);
	m_Body->SetupAttachment(m_Capsule);
	m_Gun->SetupAttachment(m_Body);
	m_Camera->SetupAttachment(m_Capsule);
	m_Arms->SetupAttachment(m_Camera);
	m_ArmsGun->SetupAttachment(m_Arms);
	m_Sphere1->SetupAttachment(m_ArmsGun);
	m_Sphere2->SetupAttachment(m_ArmsGun);
	m_Grabsocket->SetupAttachment(m_Sphere1);
	m_PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
	//m_ArmsGun->AttachToComponent(m_Arms, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	
}

// Called when the game starts or when spawned
void ABodyClone::BeginPlay()
{
	Super::BeginPlay();
	
	
	m_CloneCam = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), m_Camera->GetComponentLocation(), m_Camera->GetComponentRotation());
	m_CloneCam->GetCameraComponent()->bConstrainAspectRatio = false;
	FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
	m_CloneCam->AttachToComponent(m_Camera, Rule);
	m_CloneCam->SetActorEnableCollision(false);
	m_Body->SetVisibility(false, true);

	m_PreDotResult = false;
}

// Called every frame
void ABodyClone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsValid(m_Original)) {
		if (m_Original->GetCloneAlive()) {
			m_Camera->SetRelativeRotation(m_Original->GetFollowCamera()->GetRelativeRotation());

			m_CloneCam->SetActorLocation(m_Camera->GetComponentLocation());
			m_CloneCam->SetActorRotation(m_Camera->GetComponentRotation());
			if (m_Original->GetDotResult() != m_PreDotResult) {
				m_PreDotResult = m_Original->GetDotResult();
				if (m_PreDotResult) {
					GetWorld()->GetFirstPlayerController()->SetViewTargetWithBlend(m_CloneCam,0.f, EViewTargetBlendFunction::VTBlend_PreBlended);
				}
				else {
					GetWorld()->GetFirstPlayerController()->SetViewTargetWithBlend(UGameplayStatics::GetPlayerCharacter(GetWorld(),0), 0.f, EViewTargetBlendFunction::VTBlend_PreBlended);
				}
			}
		}
	}
}

void ABodyClone::UpdateAnim()
{
	if (m_Original->GetCloneAlive()) {
		m_ArmsGun->AttachToComponent(m_Arms, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
		m_Body->SetMasterPoseComponent(m_Original->GetCharacterMesh(),true);
		m_Gun->SetMasterPoseComponent(m_Original->GetVisualGunMesh(), true);
		m_Arms->SetMasterPoseComponent(m_Original->GetArmsMesh(), true);
		m_ArmsGun->SetMasterPoseComponent(m_Original->GetGunMesh(), true);
	}
}

