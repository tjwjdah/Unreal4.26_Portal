// Fill out your copyright notice in the Description page of Project Settings.


#include "Portal.h"
#include "../Character/PP3Character.h"
#include "../Obj/Grab_Obj.h"
// Sets default values
APortal::APortal()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	m_DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SetRootComponent(m_DefaultSceneRoot);
	m_PortalPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalPlane"));
	m_PortalPlane->SetupAttachment(m_DefaultSceneRoot);

	m_FakeRecursion = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FakeRecursion"));
	m_FakeRecursion->SetupAttachment(m_DefaultSceneRoot);

	m_CylinderTeleport = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CylinderTeleport"));
	m_CylinderTeleport->SetupAttachment(m_DefaultSceneRoot);

	m_CollisionEnabler = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollisionEnabler"));
	m_CollisionEnabler->SetupAttachment(m_DefaultSceneRoot);

	m_CollisionDesabler = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollisionDesabler"));
	m_CollisionDesabler->SetupAttachment(m_DefaultSceneRoot);

	m_CollisionCloner = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollisionCloner"));
	m_CollisionCloner->SetupAttachment(m_DefaultSceneRoot);

	m_Membrane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Membrane"));
	m_Membrane->SetupAttachment(m_DefaultSceneRoot);

	m_Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	m_Arrow->SetupAttachment(m_DefaultSceneRoot);
	
	
	static ConstructorHelpers::FObjectFinder<UMaterial>	Mtrl(TEXT("Material'/Game/Portal/M_OvalPortal.M_OvalPortal'"));
	if (Mtrl.Succeeded())
		m_PortalMaterialInit = Mtrl.Object;

	static ConstructorHelpers::FObjectFinder<UMaterial>	Mtrl2(TEXT("Material'/Game/Portal/M_OvalFakePortal.M_OvalFakePortal'"));
	if (Mtrl.Succeeded())
		m_PortalFakeMaterialInit = Mtrl2.Object;
	
	m_Decal = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
	m_Decal->SetupAttachment(m_DefaultSceneRoot);
	m_CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureComponent"));
	m_CaptureComponent->SetupAttachment(m_DefaultSceneRoot);
	m_AnotherPortal = nullptr;
	m_Support = nullptr;
	m_bSharedSupport = false;
	m_Player = nullptr;

	m_SpawnNum = SpawnNum++;
	
	m_CylinderTeleport->OnComponentBeginOverlap.AddDynamic(this, &APortal::Teleport);
	m_CollisionDesabler->OnComponentBeginOverlap.AddDynamic(this, &APortal::WallPass);
	m_CollisionCloner->OnComponentBeginOverlap.AddDynamic(this, &APortal::Clone);
	m_CollisionEnabler->OnComponentEndOverlap.AddDynamic(this, &APortal::ResetCollsion);
	m_CollisionDesabler->OnComponentEndOverlap.AddDynamic(this, &APortal::ResetCollsion);
	
}

// Called when the game starts or when spawned
void APortal::BeginPlay()
{
	Super::BeginPlay();

	m_PortalRenderTexture = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), 1920/2, 1080/2, ETextureRenderTargetFormat::RTF_RGBA8);
	m_PortalMaterial = m_PortalPlane->CreateDynamicMaterialInstance(0, m_PortalMaterialInit);
	m_PortalMaterial->SetTextureParameterValue(TEXT("ConnectedRender"), m_PortalRenderTexture);
	m_FakeRecursionMaterial = m_FakeRecursion->CreateDynamicMaterialInstance(0, m_PortalFakeMaterialInit);
	m_FakeRecursion->SetVisibleInSceneCaptureOnly(true);
	m_FakeRecursionMaterial->SetTextureParameterValue(TEXT("ConnectedRender"), m_PortalRenderTexture);
	if (m_Type == EPortalType::POUT) {
		m_FakeRecursionMaterial->SetVectorParameterValue(TEXT("BorderColor"), FVector(1.0f, 0.0275f, 0.0f));
		m_PortalMaterial->SetVectorParameterValue(TEXT("BorderColor"), FVector(1.0f, 0.0275f, 0.0f));
	}
	else
		if (m_Type == EPortalType::PIN) {
			m_FakeRecursionMaterial->SetVectorParameterValue(TEXT("BorderColor"), FVector(0.0f, 0.91f, 1.0f));
			m_PortalMaterial->SetVectorParameterValue(TEXT("BorderColor"), FVector(0.0f, 0.91f, 1.0f));
		}
	m_CylinderTeleport->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InitSupportCollisionPreset();
	
	PortalInit(this);
	/*
	FTimerHandle WaitHandle;
	GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]() {
		PortalInit(m_AnotherPortal);
		}), 0.2f, false);
	*/
	
	OnDestroyed.AddDynamic(this, &APortal::RemoveClones);
}

void APortal::InitSupportCollisionPreset()
{
	if (m_Support) {
		SharedSupport();
		if (m_bSharedSupport) 
		{
			m_SupportCollisionObjectType = Cast<UPrimitiveComponent>(m_Support->GetComponentByClass(UPrimitiveComponent::StaticClass()))->GetCollisionObjectType();
			m_NewSupportCollisionType = m_SupportCollisionObjectType;
			m_Channel = m_NewSupportCollisionType;
			InitTagSupport();
		} 
		else 
		{
			TArray<AActor*> FoundActors;
			UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("SP_Init"), FoundActors);
			TArray<TEnumAsByte<EObjectTypeQuery>> SP=m_SP;
			for (auto Act : FoundActors) {
				for(int32 i = 0; i < m_SP.Num();i++){
					if (ConvertCollisionPreset(m_SP[i]) ==
						Cast<UPrimitiveComponent>(Act->GetComponentByClass(UPrimitiveComponent::StaticClass()))->GetCollisionObjectType())
					{
						SP.Remove(m_SP[i]);
					}
				}
			}
			m_SP = SP;
			m_SupportCollisionObjectType = Cast<UPrimitiveComponent>(m_Support->GetComponentByClass(UPrimitiveComponent::StaticClass()))->GetCollisionObjectType();
			
			ConvertCollisionPreset(m_SP[0]);
			m_NewSupportCollisionType = m_Channel;
			InitTagSupport();
			
		}
	}
	
}

void APortal::SharedSupport()
{
	if (IsValid(m_AnotherPortal)) {
		if (m_AnotherPortal->m_Support == m_Support) {
			m_bSharedSupport = true;
		}
	}
	/*
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APortal::StaticClass(), FoundActors);
	for (auto Actor : FoundActors) {
		APortal* Por = Cast<APortal>(Actor);
		if (Por->m_Support == _Support && this != Actor) {
			m_bSharedSupport = true;
			break;
		}
	}
	*/
}

void APortal::BindOnDestroy()
{
	
}

void APortal::PortalInit(APortal* _portal)
{
	if (IsValid(_portal)) {
		_portal->m_CylinderTeleport->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		_portal->m_CollisionDesabler->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		_portal->m_CollisionEnabler->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

void APortal::InitTagSupport()
{
	Cast<UPrimitiveComponent>(m_Support->GetComponentByClass(UPrimitiveComponent::StaticClass()))->SetCollisionObjectType(m_Channel);
	m_Support->Tags.Add(TEXT("SP_Init"));
}

void APortal::RemoveClones(AActor* _Act)
{

	if (m_Player->m_WalkThroughtPortal == this ) {
		m_Player->m_bCloneAlive = false;
	}

	if (IsValid(m_AnotherPortal)) {
		m_AnotherPortal->m_AnotherPortal = nullptr;
	}

	m_PortalRenderTexture;
}

ECollisionChannel APortal::ConvertCollisionPreset(EObjectTypeQuery _ObjectTypeQuery)
{
    UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel2);
	switch (_ObjectTypeQuery)
	{
	case ObjectTypeQuery8:
		m_Channel = ECollisionChannel::ECC_GameTraceChannel2;
		break;
	case ObjectTypeQuery10:
		m_Channel = ECollisionChannel::ECC_GameTraceChannel4;
		break;
	case ObjectTypeQuery11:
		m_Channel = ECollisionChannel::ECC_GameTraceChannel5;
		break;
	case ObjectTypeQuery12:
		m_Channel = ECollisionChannel::ECC_GameTraceChannel6;
		break;
	case ObjectTypeQuery13:
		m_Channel = ECollisionChannel::ECC_GameTraceChannel7;
		break;
	case ObjectTypeQuery14:
		m_Channel = ECollisionChannel::ECC_GameTraceChannel8;
		break;
	case ObjectTypeQuery15:
		m_Channel = ECollisionChannel::ECC_GameTraceChannel9;
		break;
	case ObjectTypeQuery16:
		m_Channel = ECollisionChannel::ECC_GameTraceChannel10;
		break;
	}
	return m_Channel;
}

void APortal::UpdateDeltaRotator()
{
	if (IsValid(m_AnotherPortal)) {
		
		m_DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(GetActorRotation(),
			FRotator(m_AnotherPortal->GetActorRotation().Pitch,
				m_AnotherPortal->GetActorRotation().Yaw - 180.f,
				m_AnotherPortal->GetActorRotation().Roll));
	}
}

void APortal::UpdatePortalRender()
{
	if (IsValid(UGameplayStatics::GetPlayerController(GetWorld(), 0))) {
		const float fmin = 75.f;
		const float fmax = 100.f;
		if(IsValid(m_AnotherPortal)){
			bool BothTop = GetActorRotation().Pitch > -100.f && GetActorRotation().Pitch < -75.f &&
				m_AnotherPortal->GetActorRotation().Pitch > -100.f && m_AnotherPortal->GetActorRotation().Pitch < -75.f;;
			bool Bothbottom = GetActorRotation().Pitch > fmin && GetActorRotation().Pitch < fmax&&
				m_AnotherPortal->GetActorRotation().Pitch > fmin && m_AnotherPortal->GetActorRotation().Pitch < fmax;
			bool Bothhorizontal = UKismetMathLibrary::Abs(GetActorRotation().Pitch) > fmin && UKismetMathLibrary::Abs(GetActorRotation().Pitch) < fmax &&
				UKismetMathLibrary::Abs(m_AnotherPortal->GetActorRotation().Pitch) > fmin && UKismetMathLibrary::Abs(m_AnotherPortal->GetActorRotation().Pitch) < fmax;
			
			if (Bothhorizontal) {
				if (BothTop || Bothbottom) {
					//둘다 같은 바닥이나 천장
					m_CaptureComponent->SetWorldRotation(FRotator(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetControlRotation().Pitch - 180.f,
						UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetControlRotation().Yaw-180.f,
						UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetControlRotation().Roll));
				}
				else {
					//하나는 바닥 하나는 천장
					m_CaptureComponent->SetWorldRotation(FRotator(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetControlRotation()));
				}
			}
			else {
				if (UKismetMathLibrary::Abs(m_AnotherPortal->GetActorRotation().Pitch) > fmin &&
					UKismetMathLibrary::Abs(m_AnotherPortal->GetActorRotation().Pitch) < fmax) {
					//하나는 바닥 하나는 벽
					
					if(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetControlRotation().Vector().Y * m_CaptureComponent->GetForwardVector().Y > 0){
						m_CaptureComponent->SetWorldRotation(FRotator(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetControlRotation().Pitch,
							UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetControlRotation().Yaw,
							UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetControlRotation().Roll));
					}
					else {
						m_CaptureComponent->SetWorldRotation(FRotator(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetControlRotation().Pitch,
							UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetControlRotation().Yaw * -1.f,
							UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetControlRotation().Roll));
					}
				
				}
				else {
					//둘다 벽
					m_CaptureComponent->SetWorldRotation(FRotator(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetControlRotation().Pitch+ m_DeltaRotator.Pitch,
						UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetControlRotation().Yaw + m_DeltaRotator.Yaw,
						UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetControlRotation().Roll));
				}
			}
		}
	}
}

void APortal::Teleport(UPrimitiveComponent* OverlapCom, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,const FHitResult& SweepResult)
{
	
	m_SavedVelocity = FVector(0.f, 0.f, 0.f);
	m_FinalVelocity = FVector(0.f, 0.f, 0.f);
	APortal* m_OtherPortal = Cast<APortal>(OtherActor);

	if (m_OtherPortal) {
		if(m_OtherPortal->m_SpawnNum < m_SpawnNum)
			DestroyOtherPortal(m_OtherPortal);
	}
	if (IsValid(m_AnotherPortal)) {
		ACharacter* Ch = Cast<ACharacter>(OtherComp->GetOwner());
		if (Ch) {
			if (OtherComp->ComponentHasTag(TEXT("Tag_Cam"))){
				if (CanTeleport(m_AnotherPortal, Ch)) {
					m_MinimumVelocity = 400.f;
					TeleportPlayer(Ch);
				}
			}
		}
		else {
			if (OtherComp->ComponentHasTag(TEXT("Tag_Cube"))) {
				if (CanTeleport(m_AnotherPortal, OtherComp->GetOwner())) {
					if (Cast<AGrab_Obj>(OtherComp->GetOwner())->GetGrabbing())
						return;
					m_MinimumVelocity = 400.f;
					TeleportObjects(OtherComp->GetOwner());
				}
			}
		}
	}
	
}

void APortal::WallPass(UPrimitiveComponent* OverlapCom, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsValid(m_AnotherPortal)) {
		UCapsuleComponent* Cap = Cast<UCapsuleComponent>(OtherComp);
		if (Cap) {
			ACharacter* C = Cast<ACharacter>(Cap->GetOwner());
			if (C) {
				C->GetCapsuleComponent()->SetCollisionResponseToChannel(m_NewSupportCollisionType,ECollisionResponse::ECR_Ignore);
			}
		}
		else {
			OtherComp->SetCollisionResponseToChannel(m_NewSupportCollisionType, ECollisionResponse::ECR_Ignore);
		}
	}
	
}

void APortal::Clone(UPrimitiveComponent* OverlapCom, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APP3Character* Char = Cast<APP3Character>(OtherComp->GetOwner());
	if (Char) {
		Char->m_WalkThroughtPortal = this;
		if (m_LastTeleportedActor != Char) {
			Char->m_bCloneAlive = true;
			Char->CloneManagement();
		}
	}
	else{
		AGrab_Obj* Obj = Cast<AGrab_Obj>(OtherComp->GetOwner());
		if (Obj) {
			Obj->SetWalkThroughtPortal(this);
			if (m_LastTeleportedActor != Obj) {
				Obj->SetCloneAlive(true);
				Obj->CloneManagement();
			}
		}
	}
}

void APortal::ResetCollsion(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IsValid(m_AnotherPortal)) {
		UCapsuleComponent* Cap = Cast<UCapsuleComponent>(OtherComp);
		if (Cap) {
			ACharacter* C = Cast<ACharacter>(Cap->GetOwner());
			if (C) {
				C->GetCapsuleComponent()->SetCollisionResponseToChannel(m_NewSupportCollisionType, ECollisionResponse::ECR_Block);
				C->GetCapsuleComponent()->SetCollisionResponseToChannel(m_AnotherPortal->m_NewSupportCollisionType, ECollisionResponse::ECR_Block);
			}
		}
		else {
			OtherComp->SetCollisionResponseToChannel(m_NewSupportCollisionType, ECollisionResponse::ECR_Block);
			OtherComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
			OtherComp->SetCollisionResponseToChannel(m_AnotherPortal->m_NewSupportCollisionType, ECollisionResponse::ECR_Block);
			
		}
	}
	
}

void APortal::TeleportPlayer(ACharacter* _Player)
{
	m_SavedVelocity = UKismetMathLibrary::Vector_ClampSizeMax(_Player->GetVelocity(), 2500.f);
	m_AnotherPortal->m_LastTeleportedActor = _Player;
	m_Velocity = _Player->GetCharacterMovement()->Velocity;
	_Player->SetActorLocation(m_AnotherPortal->GetActorLocation(),false,nullptr,ETeleportType::TeleportPhysics);
	UpdatePlayerVelocity(_Player);
	UpdatePlayerRotation(_Player);
	PlayerCloneDestroy(_Player);
	UpdateTeleportationAuthorization();
}

void APortal::TeleportObjects(AActor* _Actor)
{

	m_SavedVelocity = UKismetMathLibrary::Vector_ClampSizeMax(_Actor->GetVelocity(), 1500.f);
	m_AnotherPortal->m_LastTeleportedActor = _Actor;
	_Actor->SetActorLocation(m_AnotherPortal->GetActorLocation(), false, nullptr, ETeleportType::TeleportPhysics);
	UStaticMeshComponent* PriCom = Cast<UStaticMeshComponent>(_Actor->GetComponentByClass(UStaticMeshComponent::StaticClass()));
	if (PriCom) {
		
		GetFinalVelocity();
		PriCom->SetAllPhysicsLinearVelocity(m_FinalVelocity);
		UpdateTeleportationAuthorization();
	}
}

bool APortal::CanTeleport(APortal* _OtherPortal, AActor* _Actor)
{
	return (_OtherPortal->m_LastTeleportedActor != _Actor) && (m_LastTeleportedActor != _Actor);
}

void APortal::DestroyOtherPortal(APortal* _OtherPortal)
{
		if (IsValid(_OtherPortal->m_AnotherPortal)) {
			_OtherPortal->m_AnotherPortal->m_CaptureComponent->TextureTarget = nullptr;
			_OtherPortal->m_AnotherPortal = nullptr;
		
			_OtherPortal->DestroyPortal();
		}
		else {
			_OtherPortal->DestroyPortal();
		}
}

void APortal::DestroyPortal()
{
	m_bSharedSupport = false;
	if(IsValid(m_AnotherPortal))
		m_AnotherPortal->m_AnotherPortal = nullptr;
	
	if (IsValid(m_Support)) {

		SharedSupport();

		if (!m_bSharedSupport) {
			Cast<UPrimitiveComponent>(m_Support->GetComponentByClass(UPrimitiveComponent::StaticClass()))->SetCollisionObjectType(m_SupportCollisionObjectType);
			TArray<FName> TagCopy= m_Support->Tags;
			for (auto Tag : m_Support->Tags) {
				if (UKismetStringLibrary::Contains(Tag.ToString(), TEXT("SP"))) {
					TagCopy.Remove(Tag);
				}
			}
			m_Support->Tags = TagCopy;
		}
		Destroy();
	}
	else
		if(IsValid(this))
			Destroy();
		
}

void APortal::UpdatePlayerVelocity(ACharacter* _Player)
{
	GetFinalVelocity();
	if (m_FinalVelocity.Z > 600.f && m_FinalVelocity.Z <900.f) {
		
		m_FinalVelocity.Z = 600.f;
	}
	
	_Player->GetCharacterMovement()->Velocity = m_FinalVelocity;
}

void APortal::UpdatePlayerRotation(ACharacter* _Player)
{
	if (IsValid(_Player)) {
		if (IsValid(_Player->GetController())) {
			
			if (m_DefaultSceneRoot->GetComponentRotation().Pitch > 75.f && m_DefaultSceneRoot->GetComponentRotation().Pitch < 100.f) {
				if (m_AnotherPortal->RootComponent->GetComponentRotation().Pitch > 75.f && m_AnotherPortal->RootComponent->GetComponentRotation().Pitch < 100.f) {
					//2 horizontal portals on the floor
					m_FinalPlayerRot = FRotator(_Player->GetControlRotation().Pitch, _Player->GetControlRotation().Yaw - m_DeltaRotator.Yaw, _Player->GetControlRotation().Roll);
				}
				else {
					if (UKismetMathLibrary::Abs(m_AnotherPortal->RootComponent->GetComponentRotation().Pitch) > 75.f &&
						UKismetMathLibrary::Abs(m_AnotherPortal->RootComponent->GetComponentRotation().Pitch) < 100.f) {
						//2 horizontal portals :1 on the floor 1 on the roof
						m_FinalPlayerRot = _Player->GetControlRotation();
					}
					else {
						//1 portal on the roof 1 on the wall
						m_FinalPlayerRot = FRotator(_Player->GetControlRotation().Pitch,
							m_AnotherPortal->RootComponent->GetComponentRotation().Yaw, _Player->GetControlRotation().Roll);
					}
				}
			}
			else {
				//2 portals on walls
				m_FinalPlayerRot = FRotator(_Player->GetControlRotation().Pitch-m_DeltaRotator.Pitch,
					_Player->GetControlRotation().Yaw- m_DeltaRotator.Yaw, _Player->GetControlRotation().Roll);
			}
			_Player->GetController()->SetControlRotation(m_FinalPlayerRot);
		}
	}
}

void APortal::PlayerCloneDestroy(ACharacter* _Player)
{
	APP3Character* Player = Cast<APP3Character>(_Player);
	if(Player)
		Player->m_bCloneAlive = false;
}

void APortal::UpdateTeleportationAuthorization()
{
	if (IsValid(m_AnotherPortal)) {
		FTimerHandle WaitHandle;
		GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]() {
			if (IsValid(m_AnotherPortal)) {
				m_AnotherPortal->m_LastTeleportedActor = nullptr;
			}
			}), 0.2f, false);
	}
}

void APortal::GetFinalVelocity()
{
	if (m_SavedVelocity.Size() > m_MinimumVelocity) {
		FVector FDot;
		m_FinalVelocity = UKismetMathLibrary::Dot_VectorVector(m_SavedVelocity, GetActorForwardVector())*m_AnotherPortal->GetActorForwardVector() +
			UKismetMathLibrary::Dot_VectorVector(m_SavedVelocity, GetActorRightVector()) * m_AnotherPortal->GetActorRightVector() +
			UKismetMathLibrary::Dot_VectorVector(m_SavedVelocity, GetActorUpVector()) * m_AnotherPortal->GetActorUpVector();

		m_FinalVelocity *= -1.f;
	}
	else {
		FVector AnotherVector = m_AnotherPortal->GetActorForwardVector();
		AnotherVector.Normalize();
		m_FinalVelocity = AnotherVector * 
			UKismetMathLibrary::SelectFloat(m_SavedVelocity.Size(), m_MinimumVelocity, m_SavedVelocity.Size() > m_MinimumVelocity);;
	}
	
}

// Called every frame
void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
		
	UpdateDeltaRotator();
	UpdatePortalRender();
	if (IsValid(m_AnotherPortal)) {
		m_AnotherPortal->m_CaptureComponent->ClipPlaneBase= m_AnotherPortal->m_Decal->GetComponentLocation() + (m_AnotherPortal->m_Decal->GetForwardVector() * -100.f);
		m_AnotherPortal->m_CaptureComponent->ClipPlaneNormal = m_AnotherPortal->m_CaptureComponent->GetForwardVector();
		
	}
	
	
	/*
	if (m_Player) {
		//m_CaptureComponent->SetWorldLocation(m_Player->GetActorLocation());
		FVector VAngle = m_Player->GetActorLocation() - GetActorLocation();
		VAngle.Normalize();
		float FDistance = FVector::Distance(m_Player->GetActorLocation(), GetActorLocation());
		if (FDistance >= 1000)
			FDistance = 1000.f;
		FDistance = FDistance / 2000.f;
		if (FDistance < 0.2)
			FDistance = 0.f;
		FDistance = 1.f - FDistance;

	}
	*/
}

