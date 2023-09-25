// Fill out your copyright notice in the Description page of Project Settings.


#include "Grab_Obj.h"
#include "../Portal/Portal.h"
// Sets default values
AGrab_Obj::AGrab_Obj()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	
	m_StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(m_StaticMesh);
	m_Grabbing = false;
	m_bIClone = false;
	m_bPortalInGrab = false;
}

// Called when the game starts or when spawned
void AGrab_Obj::BeginPlay()
{
	Super::BeginPlay();
	m_MaterialInstance = m_StaticMesh->CreateDynamicMaterialInstance(0, m_Material, TEXT("None"));
	m_MaterialInstance->SetVectorParameterValue(TEXT("Color"), m_LaserColor);
	m_StaticMesh->SetMaterial(0, m_MaterialInstance);

}

// Called every frame
void AGrab_Obj::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (m_bIClone && m_bPortalInGrab)
		return;
	if (m_bCloneAlive) {
		if (IsValid(m_WalkThroughtPortal)) {
			if (IsValid(m_Clone)) {
				FTransform Trans = UKismetMathLibrary::ConvertTransformToRelative(GetActorTransform(), m_WalkThroughtPortal->GetActorTransform());
				FVector ConvertVector = GetActorLocation()- m_WalkThroughtPortal->GetActorLocation();
				ConvertVector.X *= -1.f;
				//ConvertVector.X *= -1.f;
				
				FRotator ConvertRotator = FRotator(GetActorRotation().Pitch - m_WalkThroughtPortal->GetDeltaRotator().Pitch, 
					GetActorRotation().Yaw - m_WalkThroughtPortal->GetDeltaRotator().Yaw,
					GetActorRotation().Roll - m_WalkThroughtPortal->GetDeltaRotator().Roll);
				
				m_Clone->SetActorRotation(ConvertRotator);
				m_Clone->SetActorRelativeLocation(ConvertVector);
				
				//m_Clone->SetActorRelativeLocation(ConvertVector + m_Clone->GetActorForwardVector().Normalize());
			}

			if (IsValid(m_WalkThroughtPortal->GetAnotherPortal())) {
				m_bInsidePortal = false;
				TArray<AActor*> FoundActors;
				GetOverlappingActors(FoundActors, APortal::StaticClass());
				if (FoundActors.Num() > 0) {
					APortal* InPortal = Cast<APortal>(FoundActors[0]);
					if (IsValid(InPortal)) {
						TArray<UPrimitiveComponent*> FoundComponent;
						GetOverlappingComponents(FoundComponent);
						for (auto Comp : FoundComponent) {
							if (Comp == InPortal->GetCollisionCloner()) {
								m_bInsidePortal = true;
								break;
							}
						}
						if (!m_bInsidePortal && !m_Grabbing) {
							
							m_bCloneAlive = false;
						}
					}
				}
			}
			else {
				m_bCloneAlive = false;
				CloneManagement();
			}
		}
		else {
			m_bCloneAlive = false;
			CloneManagement();
		}
	}
	else {

		CloneManagement();
	}
}


void AGrab_Obj::SetForceDirection(FVector _Dir, float _Force)
{
	m_StaticMesh->AddImpulse(_Dir * _Force, TEXT("None"), true);
}

void AGrab_Obj::CloneManagement()
{
	if (m_bIClone)
		return;
	if (IsValid(m_Clone)) {
		if (!m_bCloneAlive) {
			
			//clone destroy
			m_Clone->SetActorHiddenInGame(false);
			m_WalkThroughtPortal = nullptr;
			m_Clone->Destroy();
			m_Clone = nullptr;
			
		}
	}
	else
	{
		if (m_bCloneAlive) {
			if (IsValid(m_WalkThroughtPortal) && IsValid(m_WalkThroughtPortal->GetAnotherPortal())) {
				//m_BodyClone ½ºÆù
				
				m_Clone = GetWorld()->SpawnActor<AGrab_Obj>(m_CloneBp->GetDefaultObject()->GetClass());
				m_Clone->m_bIClone = true;
				m_Clone->m_LaserColor = m_LaserColor;
				m_Clone->MaterialSet();
				m_Clone->m_StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				m_Clone->m_StaticMesh->SetSimulatePhysics(false);
				FAttachmentTransformRules Rule(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld, true);
				m_Clone->AttachToActor(m_WalkThroughtPortal->GetAnotherPortal(), Rule);
				m_ClonePitchInit = GetActorRotation().Pitch - m_WalkThroughtPortal->GetDeltaRotator().Pitch;

				FTransform Trans = UKismetMathLibrary::ConvertTransformToRelative(GetActorTransform(), m_WalkThroughtPortal->GetActorTransform());
				
				FVector ConvertVector = Trans.GetLocation() *-1.F ;
				
				FRotator ConvertRotator = FRotator(m_ClonePitchInit, GetActorRotation().Yaw - m_WalkThroughtPortal->GetDeltaRotator().Yaw,
					GetActorRotation().Roll);
				
				m_Clone->SetActorRotation(ConvertRotator);
				m_Clone->SetActorRelativeLocation(ConvertVector + GetActorForwardVector().Normalize());
			}

		}
	}
}

void AGrab_Obj::MaterialSet()
{
	m_MaterialInstance->SetVectorParameterValue(TEXT("Color"), m_LaserColor);
}

void AGrab_Obj::CloneDestroy()
{
	m_Clone->SetActorHiddenInGame(false);
	m_WalkThroughtPortal = nullptr;
	m_Clone->Destroy();
	m_Clone = nullptr;
}

