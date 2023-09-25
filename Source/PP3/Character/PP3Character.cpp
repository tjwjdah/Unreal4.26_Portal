// Copyright Epic Games, Inc. All Rights Reserved.

#include "PP3Character.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"

#include "Components/BoxComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

#include "../Portal/Portal.h"
#include "../Portal/PortalChecker.h"
#include "../Obj/Grab_Obj.h"
#include "../PP3GameMode.h"
#include "BodyClone.h"
//////////////////////////////////////////////////////////////////////////
// APP3Character

APP3Character::APP3Character()
{
	m_FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	m_Arms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Arms"));
	m_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Gun"));
	m_Sphere1 = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere1"));
	m_Sphere2 = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere2"));
	m_Arrow1 = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow1"));
	m_PortalTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("PortalTrigger"));
	m_Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	m_CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
	m_VisualGun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VisualGun"));
	m_PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
	m_Grabsocket = CreateDefaultSubobject<USceneComponent>(TEXT("Grabsocket"));
	//m_Test = CreateDefaultSubobject<USceneComponent>(TEXT("Test"));
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	

	// Create a follow camera
	
	m_FirstPersonCamera->SetupAttachment(GetCapsuleComponent()); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	m_FirstPersonCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	
	m_Arms->SetupAttachment(m_FirstPersonCamera);

	
	m_Gun->SetupAttachment(m_Arms);
	
	//m_Gun->AttachToComponent(m_Arms, FAttachmentTransformRules(EDetachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget,), TEXT("GripPoint"));
	
	m_Sphere1->SetupAttachment(m_Gun);
	m_Sphere2->SetupAttachment(m_Gun);
	
	m_Arrow1->SetupAttachment(m_Sphere1);
	m_Grabsocket->SetupAttachment(m_Arrow1);
	
	m_PortalTrigger->SetupAttachment(m_Sphere1);

	
	m_Box->SetupAttachment(m_FirstPersonCamera);

	
	m_CharacterMesh->SetupAttachment(GetCapsuleComponent());

	
	m_VisualGun->SetupAttachment(m_CharacterMesh,TEXT("Gun_Socket"));

	//m_Test->SetupAttachment(m_Sphere1);
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
	
	//m_Gun->AttachToComponent(m_Arms, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	NiagaraLase = CreateDefaultSubobject<UNiagaraSystem>(TEXT("NiagaraLase"));;
	NiagaraImpact = CreateDefaultSubobject<UNiagaraSystem>(TEXT("NiagaraImpact"));
	GrabParticle = CreateDefaultSubobject<UNiagaraSystem>(TEXT("GrabParticle"));
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> NiagaraParticle(TEXT("NiagaraSystem'/Game/MPG/NiagaraElements/FXS_Laser.FXS_Laser'"));
	if (NiagaraParticle.Succeeded())
		NiagaraLase = NiagaraParticle.Object;
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> NiagaraParticle1(TEXT("NiagaraSystem'/Game/MPG/NiagaraElements/FXS_LaserImpact.FXS_LaserImpact'"));
	if (NiagaraParticle1.Succeeded())
		NiagaraImpact = NiagaraParticle1.Object;
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> NiagaraParticle2(TEXT("NiagaraSystem'/Game/UportalSystemNiagara/Niagara/NS_GrabObject.NS_GrabObject'"));
	if (NiagaraParticle2.Succeeded())
		GrabParticle = NiagaraParticle2.Object;

	
	static ConstructorHelpers::FClassFinder<APortal>	InPortal(TEXT("Blueprint'/Game/Portal/BP_InPortal.BP_InPortal_C'"));
	if (InPortal.Succeeded())
		m_InPortalBp = InPortal.Class;
	static ConstructorHelpers::FClassFinder<APortal>	OutPortal(TEXT("Blueprint'/Game/Portal/BP_OutPortal.BP_OutPortal_C'"));
	if (OutPortal.Succeeded())
		m_OutPortalBp = OutPortal.Class;
	/*
	static ConstructorHelpers::FClassFinder<ABodyClone>	BodyCloneBp(TEXT("Blueprint'/Game/Character/m_BodyClone.m_BodyClone_C'"));
	if (BodyCloneBp.Succeeded())
	m_BodyCloneBp = BodyCloneBp.Class;
	*/

	DoOnce = true;
	m_Grabbing = false;
	m_bJumpButtonDown = false;
	m_GumMode = EGunMode::Portal;
}

//////////////////////////////////////////////////////////////////////////
// Input

void APP3Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APP3Character::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &APP3Character::StopJumping);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APP3Character::LMousePressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &APP3Character::LMouseReleased);
	PlayerInputComponent->BindAction("Fire2", IE_Pressed, this, &APP3Character::RMousePressed);
	PlayerInputComponent->BindAction("Fire2", IE_Released, this, &APP3Character::RMouseReleased);
	PlayerInputComponent->BindAction("Change", IE_Pressed, this, &APP3Character::GunModeChange);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &APP3Character::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APP3Character::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APP3Character::TurnAtRate);
	
	PlayerInputComponent->BindAxis("LookUp", this, &APP3Character::LookUpAtRate);
	

	
	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &APP3Character::OnResetVR);
}

void APP3Character::BeginPlay()
{
	Super::BeginPlay();
	APP3GameMode* GameMode = Cast<APP3GameMode>(GetWorld()->GetAuthGameMode());
	m_MainHUD = GameMode->GetMainHUD();
	
	
	IgnoroeActors.Add(this);
}

void APP3Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	
	
	if (m_bCloneAlive) {
		if (IsValid(m_WalkThroughtPortal)) {
			if (IsValid(m_BodyClone)) {
				FTransform Trans = UKismetMathLibrary::ConvertTransformToRelative(GetActorTransform(), m_WalkThroughtPortal->GetActorTransform());
				FVector ConvertVector = Trans.GetLocation() * -1.f;
				FRotator ConvertRotator = FRotator(m_ClonePitchInit, GetController()->GetControlRotation().Yaw - m_WalkThroughtPortal->GetDeltaRotator().Yaw,
					GetController()->GetControlRotation().Roll);
				//m_BodyClone->SetActorRelativeLocation(ConvertVector);
				m_BodyClone->SetActorRotation(ConvertRotator);
				m_BodyClone->SetActorRelativeLocation(ConvertVector + m_BodyClone->GetActorForwardVector().Normalize() );
			}

			//m_bDotResult
			FVector ArrowForwardVector =m_WalkThroughtPortal->GetArrow()->GetForwardVector();
			FVector CameraForwardVector = m_FirstPersonCamera->GetForwardVector();
			ArrowForwardVector.Normalize();
			CameraForwardVector.Normalize();
			
			float Dot = UKismetMathLibrary::Abs(UKismetMathLibrary::Dot_VectorVector(ArrowForwardVector, CameraForwardVector)) /
				UKismetMathLibrary::VSize(m_FirstPersonCamera->GetComponentLocation() - m_WalkThroughtPortal->GetArrow()->GetComponentLocation());
			m_bDotResult = (Dot > 0.01);
			if (!m_bDotResult) {
				m_CharacterMesh->SetVisibility(true, true);
				m_Arms->SetVisibility(true, true);
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
						if (!m_bInsidePortal) {
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

	if (m_Grabbing)
	{

		m_PhysicsHandle->SetTargetLocation(m_Grabsocket->GetComponentLocation());
		if (IsValid(m_BodyClone)) {
			m_BodyClone->GetPhysicsHandle()->SetTargetLocation(m_BodyClone->GetGrabsocket()->GetComponentLocation());
		}
	}
}

void APP3Character::CloneManagement()
{
	if (IsValid(m_BodyClone)) {
		if (!m_bCloneAlive) {

			//clone destroy
			m_BodyClone->SetActorHiddenInGame(false);
			GetWorld()->GetFirstPlayerController()->SetViewTargetWithBlend(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0), 0.f,EViewTargetBlendFunction::VTBlend_PreBlended);
			FTransform CameraT = m_BodyClone->GetCamera()->GetRelativeTransform();
			m_WalkThroughtPortal = nullptr;
			m_BodyClone->Destroy();
			m_Arms->SetVisibility(true, true);
			if (m_Grabbing) {
				m_GrabObj->SetPortalInGrab(false);
				m_GrabObj->GetStaticMesh()->SetRelativeRotation(m_GrabObj->GetClone()->GetStaticMesh()->GetRelativeRotation());
				m_GrabObj->CloneDestroy();
				
			}
		}
	}
	else 
	{
		if (m_bCloneAlive) {
			if (IsValid(m_WalkThroughtPortal) && IsValid(m_WalkThroughtPortal->GetAnotherPortal())) {
				//m_BodyClone ½ºÆù
				m_BodyClone =GetWorld()->SpawnActor<ABodyClone>(m_BodyCloneBp->GetDefaultObject()->GetClass(), GetActorTransform());
				
				m_BodyClone->SetOriginal(this);
				m_BodyClone->UpdateAnim();
				FAttachmentTransformRules Rule(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld,true);
				m_BodyClone->AttachToActor(m_WalkThroughtPortal->GetAnotherPortal(), Rule);
				m_ClonePitchInit = GetActorRotation().Pitch - m_WalkThroughtPortal->GetDeltaRotator().Pitch;

				FTransform Trans = UKismetMathLibrary::ConvertTransformToRelative(GetActorTransform(), m_WalkThroughtPortal->GetActorTransform());
				FVector ConvertVector = Trans.GetLocation() * -1.f;
				FRotator ConvertRotator = FRotator(m_ClonePitchInit, GetController()->GetControlRotation().Yaw - m_WalkThroughtPortal->GetDeltaRotator().Yaw,
					GetController()->GetControlRotation().Roll);
				//m_BodyClone->SetActorRelativeLocation(ConvertVector);
				m_BodyClone->SetActorRotation(ConvertRotator);
				m_BodyClone->SetActorRelativeLocation(ConvertVector + m_BodyClone->GetActorForwardVector().Normalize() );
				m_WalkThroughtPortal->SetPlayer(this);
				m_Arms->SetVisibility(false, true);
				if (m_Grabbing) {
					
					m_GrabParticle2 = UNiagaraFunctionLibrary::SpawnSystemAttached(GrabParticle, m_BodyClone->GetSphere2(), TEXT("None"), FVector::ZeroVector,
						FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false);
					m_GrabObj->SetPortalInGrab(true);
					m_BodyClone->GetPhysicsHandle()->GrabComponentAtLocation(m_GrabObj->GetClone()->GetStaticMesh(), TEXT("None"),
						m_GrabObj->GetClone()->GetStaticMesh()->GetComponentLocation());
					
					
				}
			}
			
		}
	}
}


void APP3Character::OnResetVR()
{
	// If PP3 is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in PP3.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}



void APP3Character::Jump()
{
	Super::Jump();
}

void APP3Character::StopJumping()
{
	Super::StopJumping();
}

void APP3Character::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate);
}

void APP3Character::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate  );

	m_FirstPersonCamera->SetWorldRotation(GetControlRotation());
}

void APP3Character::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		const FVector Direction = GetActorForwardVector();
		AddMovementInput(Direction, Value);
	}
}

void APP3Character::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
	
		// get right vector 
		const FVector Direction = GetActorRightVector();
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
void APP3Character::Check2(bool _FinalLocationCheck) {
	//TArray<FHitResult>	HitResultArray;
	FVector CheckDirection;
	FVector CheckCDirection;
	m_ActorsIgnore.Empty();
	for (int i = 0; i < 4; i++) {
		//HitResultArray.Empty();
		switch (i)
		{
		case 0:
			CheckDirection = m_PortalChecker->GetUpCheck()->GetComponentLocation();
			CheckCDirection = m_PortalChecker->GetDownCheck()->GetComponentLocation();
			break;
		case 1:
			CheckDirection = m_PortalChecker->GetDownCheck()->GetComponentLocation();
			CheckCDirection = m_PortalChecker->GetUpCheck()->GetComponentLocation();
			break;
		case 2:
			CheckDirection = m_PortalChecker->GetLeftCheck()->GetComponentLocation();
			CheckCDirection = m_PortalChecker->GetRightCheck()->GetComponentLocation();
			break;
		case 3:
			CheckDirection = m_PortalChecker->GetRightCheck()->GetComponentLocation();
			CheckCDirection = m_PortalChecker->GetLeftCheck()->GetComponentLocation();
			break;
		}
		FHitResult result;
		/*UKismetSystemLibrary::BoxTraceMulti(GetWorld(), CheckDirection->GetComponentLocation(), CheckDirection->GetComponentLocation(), FVector(10.f, 10.f, 10.f), m_PortalChecker->GetActorRotation(),
			ETraceTypeQuery::TraceTypeQuery1, false, m_ActorsIgnore, EDrawDebugTrace::None, HitResultArray, true);*/
		/*
		FCollisionQueryParams	params(NAME_None, false,this);
		GetWorld()->SweepSingleByChannel(result, m_Arrow1->GetComponentLocation(), CheckDirection, FQuat::Identity, ECollisionChannel::ECC_GameTraceChannel1,
			FCollisionShape::MakeSphere(20.f), params);
		
		*/
		FVector vector = FVector(30.f, 30.f, 30.f) * m_PortalChecker->GetActorForwardVector();
		UKismetSystemLibrary::LineTraceSingle(GetWorld(), CheckDirection +  vector, CheckDirection - vector
			, ETraceTypeQuery::TraceTypeQuery1, false, IgnoroeActors, EDrawDebugTrace::None, result, true);
		
		if (_FinalLocationCheck) {
			if (result.bBlockingHit) {
				if (result.Actor != m_PortalBeamHitActor)
				{
					
					m_PortalChecker->SetActorLocation(CheckCDirection);

				}
			}
			else {
				
				m_PortalChecker->SetActorLocation(CheckCDirection);
			}
		}
		else 
		{
			if (result.bBlockingHit) {
				
				if (result.Actor != m_PortalBeamHitActor)
				{
					
					m_bPortalAutorisation = false;
				}
			}
			else {
				if (!_FinalLocationCheck)
				
				m_bPortalAutorisation = false;
			}
		} 
	}
	
}

void APP3Character::Check(bool _OtherObjectDetection,EDirection _Direction)
{
	USceneComponent* CheckDirection  = nullptr;
	USceneComponent* CheckCDirection = nullptr;
	switch (_Direction)
	{
	case EDirection::Up:
		CheckDirection = m_PortalChecker->GetUpCheck();
		CheckCDirection = m_PortalChecker->GetDownCheck();
		break;
	case EDirection::Down:
		CheckDirection = m_PortalChecker->GetDownCheck();
		CheckCDirection = m_PortalChecker->GetUpCheck();
		break;
	case EDirection::Left:
		CheckDirection = m_PortalChecker->GetLeftCheck();
		CheckCDirection = m_PortalChecker->GetRightCheck();
		break;
	case EDirection::Right:
		CheckDirection = m_PortalChecker->GetRightCheck();
		CheckCDirection = m_PortalChecker->GetLeftCheck();
		break;
	}

	m_ActorsIgnore.Empty();
	if (_OtherObjectDetection) {
		m_ActorsIgnore.Add(m_PortalBeamHitActor);
	}
	FHitResult result;
	UKismetSystemLibrary::SphereTraceSingle(GetWorld(), CheckDirection->GetComponentLocation(), CheckDirection->GetComponentLocation(), m_CheckerRadius,
		ETraceTypeQuery::TraceTypeQuery1,false,m_ActorsIgnore,EDrawDebugTrace::None, result,true);
	if (result.bBlockingHit) {
		if (result.Actor != m_PortalBeamHitActor) {
			m_bPortalAutorisation = false;
			m_PortalChecker->SetActorLocation(CheckCDirection->GetComponentLocation());
		}
	}
	else {
		if (!_OtherObjectDetection) {
			m_bPortalAutorisation = false;
			m_PortalChecker->SetActorLocation(CheckCDirection->GetComponentLocation());
		}
	}
}


void APP3Character::Check360()
{
	Check(false, EDirection::Up);
	Check(true, EDirection::Up);
	Check(false, EDirection::Left);
	Check(true, EDirection::Left);
	Check(false, EDirection::Down);
	Check(true, EDirection::Down);
	Check(false, EDirection::Right);
	Check(true, EDirection::Right);
}


void APP3Character::PlayerPortalSound()
{
	
	if (DoOnce) {
		DoOnce = false;
		if (!IsValid(m_Sound)) 
		{
			m_Sound = LoadObject<USoundBase>(nullptr, TEXT("SoundCue'/Game/Sounds/SC_Laser.SC_Laser'"));
		}
		
		if (m_Sound) {
			
			m_LaserSound=UGameplayStatics::SpawnSoundAtLocation(GetWorld(), m_Sound, m_Arrow1->GetComponentLocation());
			if(IsValid(m_LaserSound))
				m_LaserSound->AttachToComponent(m_Arrow1, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true));
			
		}
	}
}

void APP3Character::SoundStop()
{
	m_LaserSound->DestroyComponent();
	DoOnce = true;
}

void APP3Character::FireStop()
{
	m_bFiring = false;
	if (IsValid(m_Laser) && IsValid(m_LaserImpact)) {
		m_Laser->DestroyComponent();
		m_LaserImpact->DestroyComponent();
		SoundStop();
	}
}

void APP3Character::FireStart(EPortalType _PortalType)
{
	
	//m_Laser = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NiagaraLase, m_Arrow1->GetComponentLocation());
	m_Laser = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NiagaraLase, m_Arrow1->GetComponentLocation());
	
	m_LaserImpact = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NiagaraImpact, m_Arrow1->GetComponentLocation());

	if (IsValid(m_Laser) && IsValid(m_LaserImpact)) {
		m_Laser->SetColorParameter(TEXT("Color"), m_LaserColor);
		m_LaserImpact->SetColorParameter(TEXT("Color"), m_LaserColor);
		
		FHitResult	result;
		
		
		
		UKismetSystemLibrary::LineTraceSingle(GetWorld(), m_Arrow1->GetComponentLocation(), m_Arrow1->GetComponentLocation() + (m_FirstPersonCamera->GetForwardVector() * m_LaserDistance)
			, ETraceTypeQuery::TraceTypeQuery1, false, IgnoroeActors, EDrawDebugTrace::None, result, true);
		
		
		m_bLaserImpact = result.bBlockingHit;
		
		if (IsValid(m_Laser)) {
			//m_Laser->SetWorldLocation(m_Arrow1->GetComponentLocation());
			FVector Point;
			if (m_bLaserImpact)
				Point = result.ImpactPoint;
			else
				Point = m_Arrow1->GetComponentLocation() + (m_FirstPersonCamera->GetForwardVector() * m_LaserDistance);
			
			m_Laser->SetVectorParameter(TEXT("LaserEnd"), Point);
			m_LaserImpact->SetWorldLocation(Point);
			m_LaserImpact->SetVisibility(m_bLaserImpact);
			PlayerPortalSound();
			FTimerHandle WaitHandle;
			GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]() {
				FireStop();
				}), 0.2f, false);

			SpawnPortal(_PortalType, result);
		}
	}
	

}

void APP3Character::SpawnPortal(EPortalType _PortalType, FHitResult _HitResult)
{
	m_PortalType = _PortalType;
	
	if (_HitResult.bBlockingHit) {
		m_PortalBeamHitActor = _HitResult.Actor.Get();
		m_PortalFinalLocation = _HitResult.Location;
		FRotator Rot = UKismetMathLibrary::MakeRotFromXZ(_HitResult.Normal, _HitResult.Normal);
		
		m_PortalChecker = GetWorld()->SpawnActor<APortalChecker>(APortalChecker::StaticClass(), _HitResult.Location, Rot);
		m_PortalChecker->SetPortalSupport(m_PortalBeamHitActor);
		
		
		//Check360();
		Check2(true);
		m_PortalFinalLocation = m_PortalChecker->GetActorLocation();
		m_bPortalAutorisation = true;
		//Check360();
		Check2(false);
		
		//m_PortalChecker->Destroy();
		
		if (m_bPortalAutorisation) {
			if (IsValid(m_InPortal) && m_InPortal->GetType() == m_PortalType){
				m_InPortal->DestroyPortal();
				m_InPortal = nullptr;
			}
			if (IsValid(m_OutPortal) && m_OutPortal->GetType() == m_PortalType) {
				m_OutPortal->DestroyPortal(); 
				m_OutPortal = nullptr;
			}
			FTransform SpawnTransform(Rot,m_PortalFinalLocation );
			if (m_PortalType == EPortalType::PIN)
			{
				m_InPortal = GetWorld()->SpawnActorDeferred<APortal>(m_InPortalBp->GetDefaultObject()->GetClass(), SpawnTransform);
				m_InPortal->SetSupport(m_PortalBeamHitActor);
				m_InPortal->SetPlayer(this);
				if (IsValid(m_OutPortal)) {
					m_InPortal->SetAnotherPortal(m_OutPortal);
					m_OutPortal->SetAnotherPortal(m_InPortal);
				}
				m_InPortal->FinishSpawning(SpawnTransform);
				if (IsValid(m_OutPortal))
				{
					m_InPortal->GetCaptureComponent()->TextureTarget = m_OutPortal->GetPortalRenderTexture();
					m_OutPortal->GetCaptureComponent()->TextureTarget = m_InPortal->GetPortalRenderTexture();
				}
			}
			else
			{
				
				m_OutPortal = GetWorld()->SpawnActorDeferred<APortal>(m_OutPortalBp->GetDefaultObject()->GetClass(), SpawnTransform);
				m_OutPortal->SetSupport(m_PortalBeamHitActor);
				m_OutPortal->SetPlayer(this);
				
				if (IsValid(m_InPortal))
				{
					m_OutPortal->SetAnotherPortal(m_InPortal);
					m_InPortal->SetAnotherPortal(m_OutPortal);
					
				}
				m_OutPortal->FinishSpawning(SpawnTransform);
				if (IsValid(m_InPortal))
				{
					m_InPortal->GetCaptureComponent()->TextureTarget = m_OutPortal->GetPortalRenderTexture();
					m_OutPortal->GetCaptureComponent()->TextureTarget = m_InPortal->GetPortalRenderTexture();
				}
			}
		}
	}
}

void APP3Character::GrabObject()
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectType;
	ObjectType.Add(EObjectTypeQuery::ObjectTypeQuery3);
	
	FHitResult	result;
	UKismetSystemLibrary::CapsuleTraceSingleForObjects(GetWorld(), GetActorLocation(), GetActorLocation() + GetActorForwardVector() * 2000.f,
		42.0, 96.0f, ObjectType, false, IgnoroeActors, EDrawDebugTrace::None, result, true);
	if (result.bBlockingHit) {
		USoundBase* LSound = LoadObject<USoundBase>(nullptr, TEXT("SoundWave'/Game/UportalSystemNiagara/Sounds/S_trailFire.S_trailFire'"));
		if (LSound)
			UGameplayStatics::PlaySound2D(GetWorld(), LSound);
		
		m_GrabParticle=UNiagaraFunctionLibrary::SpawnSystemAttached(GrabParticle, m_Sphere2, TEXT("None"),FVector::ZeroVector,
			FRotator::ZeroRotator,EAttachLocation::KeepRelativeOffset,false);
		/*m_GrabParticle = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), GrabParticle, m_Sphere1->GetComponentLocation());
		*/
		m_GrabObj = Cast<AGrab_Obj>(result.Actor.Get());
		if (m_GrabObj) {
			m_GrabObj->SetGrabbing(true);
			m_Grabbing = true;
			
			m_GrabObjMesh = result.Component.Get();
			
			m_PhysicsHandle->GrabComponentAtLocation(m_GrabObjMesh, TEXT("None"), m_GrabObjMesh->GetComponentLocation());
		}
	}
	else {
		USoundBase* LSound = LoadObject<USoundBase>(nullptr, TEXT("SoundWave'/Game/UportalSystemNiagara/Sounds/S_none.S_none'"));
		if (LSound)
			UGameplayStatics::PlaySound2D(GetWorld(), LSound);
	}

}

void APP3Character::GrabRelease()
{
	if (IsValid(m_GrabParticle)) {
		m_GrabParticle->DestroyComponent();
		if (IsValid(m_BodyClone)) {
			m_GrabParticle2->DestroyComponent();
		}
	}
	if (IsValid(m_GrabObj)) {
		m_GrabObj->SetGrabbing(false);
		m_Grabbing = false;
		USoundBase* LSound = LoadObject<USoundBase>(nullptr, TEXT("SoundWave'/Game/UportalSystemNiagara/Sounds/S_fire_2.S_fire_2'"));
		if (LSound)
			UGameplayStatics::PlaySound2D(GetWorld(), LSound);
		m_PhysicsHandle->ReleaseComponent();
		m_GrabObj = nullptr;
		m_GrabObjMesh = nullptr;

		if (IsValid(m_BodyClone)) {
			m_BodyClone->GetPhysicsHandle()->ReleaseComponent();
			
		}
	}
	
}

void APP3Character::GrabLaunch()
{
	if (IsValid(m_GrabParticle)) {
		m_GrabParticle->DestroyComponent();
		if (IsValid(m_BodyClone)) {
			m_GrabParticle2->DestroyComponent();
		}
	}
	if (IsValid(m_GrabObj)) {
		m_GrabObj->SetGrabbing(false);
		m_Grabbing = false;
		USoundBase* LSound = LoadObject<USoundBase>(nullptr, TEXT("SoundWave'/Game/UportalSystemNiagara/Sounds/S_fire_2.S_fire_2'"));
		if (LSound)
			UGameplayStatics::PlaySound2D(GetWorld(), LSound);
		m_PhysicsHandle->ReleaseComponent();
		m_GrabObj->SetForceDirection(UKismetMathLibrary::GetForwardVector(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetControlRotation()), 2000.f);
		m_GrabObj = nullptr;
		m_GrabObjMesh = nullptr;

		if (IsValid(m_BodyClone)) {
			m_BodyClone->GetPhysicsHandle()->ReleaseComponent();
		}
	}
}


void APP3Character::Fire()
{
	if (!m_bCloneAlive) {
		m_bFiring = false;
		FireStop();
		if (m_bCanShoot) {
			m_LaserColor = FLinearColor(FVector(0.f, 1.f, 0.987f));
			
			m_bFiring = true;
			FireStart(EPortalType::PIN);
			m_bCanShoot = false;
		}
	}
}

void APP3Character::FireReleased()
{
	m_bCanShoot = true;
	FireStop();
}

void APP3Character::Fire2()
{
	if (!m_bCloneAlive) {
		m_bFiring = false;
		FireStop();
		if (m_bCanShoot) {
			m_LaserColor = FLinearColor(FVector(0.989583f, 0.010216f, 0.f));
			
			m_bFiring = true;
			FireStart(EPortalType::POUT);
			m_bCanShoot = false;
		}
	}
}


void APP3Character::GunModeChange()
{
	if (m_GumMode == EGunMode::Portal)
	{
		m_GumMode = EGunMode::Grab;
		
	}
	else {
		m_GumMode = EGunMode::Portal;
		
	}
	m_MainHUD->GetGunMode()->SetMode(m_GumMode);
}


void APP3Character::LMousePressed()
{
	if (m_GumMode == EGunMode::Portal)
	{
		Fire();
	}
	else {
		GrabObject();
	}
}

void APP3Character::RMousePressed()
{
	if (m_GumMode == EGunMode::Portal)
	{
		Fire2();
	}
	else {
			GrabLaunch();
	}
}
void APP3Character::LMouseReleased()
{
	if (m_GumMode == EGunMode::Portal)
	{
		FireReleased();
	}
	else {
		GrabRelease();
	}
}

void APP3Character::RMouseReleased()
{
	if (m_GumMode == EGunMode::Portal)
	{
		FireReleased();
	}
	else {

	}
}
