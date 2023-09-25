// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "../GameInfo.h"
#include "GameFramework/Character.h"
#include "PP3Character.generated.h"

class APortal;
class ABodyClone;



UCLASS(config=Game)
class APP3Character : public ACharacter
{
	GENERATED_BODY()

private:



	/** Camera boom positioning the camera behind the character */
	
	/** Follow camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = MyCharacter)
	UCameraComponent* m_FirstPersonCamera;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = MyCharacter)
	USkeletalMeshComponent* m_Arms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Clone)
		class ABodyClone* m_BodyClone;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Clone)
		bool m_bCloneAlive;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Clone)
		float m_ClonePitchInit;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Clone)
		bool m_bDotResult;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Grab)
		UNiagaraComponent* m_GrabParticle2;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = placement)
		TArray<AActor*> m_ActorsIgnore;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = placement)
		AActor* m_PortalBeamHitActor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = placement)
		FVector m_PortalFinalLocation;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Control)
		bool m_bJumpButtonDown;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = GunAndLaser)
		bool m_bFiring;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = GunAndLaser)
		bool m_bLaserImpact;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = GunAndLaser)
		FLinearColor m_LaserColor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = GunAndLaser)
		float m_LaserDistance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = GunAndLaser)
		bool m_bCanShoot;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* m_Gun;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USphereComponent* m_Sphere1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USphereComponent* m_Sphere2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UArrowComponent* m_Arrow1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USceneComponent* m_Grabsocket;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UBoxComponent* m_PortalTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UBoxComponent* m_Box;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class USkeletalMeshComponent* m_CharacterMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class USkeletalMeshComponent* m_VisualGun;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TEnumAsByte<EGunMode> m_GumMode;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Base)
		UNiagaraComponent* m_Laser;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Base)
		UNiagaraComponent* m_LaserImpact;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Base)
		UAudioComponent* m_LaserSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Portal)
		class APortal* m_WalkThroughtPortal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Portal)
		EPortalType m_PortalType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Portal)
		class APortal* m_InPortal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Portal)
		class APortal* m_OutPortal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Portal)
		 bool m_bFoundPortal;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Portal)
		class APortalChecker* m_PortalChecker;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Portal)
		bool m_bPortalAutorisation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Portal)
		float m_CheckerRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Portal)
		bool m_bInsidePortal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Grab)
		UNiagaraComponent* m_GrabParticle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Grab)
		class AGrab_Obj* m_GrabObj;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Grab)
		UPrimitiveComponent* m_GrabObjMesh;


	bool DoOnce;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* NiagaraLase;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* NiagaraImpact;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* GrabParticle;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UPhysicsHandleComponent* m_PhysicsHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USoundBase* m_Sound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<APortal> m_InPortalBp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<APortal> m_OutPortalBp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ABodyClone> m_BodyCloneBp;

	class  UMainHUD* m_MainHUD;
	bool m_Grabbing;
	TArray<AActor*> IgnoroeActors;

	friend class APortal;

	
public:
	APP3Character();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void Jump();

	/** Handler for when a touch input stops. */
	void StopJumping();
	void GunModeChange();
	void LMousePressed();
	void RMousePressed();
	void LMouseReleased();
	void RMouseReleased();
	void Fire();
	void FireReleased();
	void Fire2();
	void GrabObject();
	void GrabRelease();
	void GrabLaunch();

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void BeginPlay() override;
public:
	
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return m_FirstPersonCamera; }



	virtual void Tick(float DeltaTime) override;


	void CloneManagement();

	bool GetJumpButtonDown() { return m_bJumpButtonDown; }

	UFUNCTION(BlueprintCallable)
	void SetJumpButtonDown(bool _JumpButtonDown) { m_bJumpButtonDown = _JumpButtonDown; }

	

	void Check(bool _OtherObjectDetection,EDirection _Direction);
	void Check2(bool _FinalLocationCheck);
	void Check360();
	
	void PlayerPortalSound();
	void SoundStop();

	void FireStop();
	void FireStart(EPortalType _PortalType);

	void SpawnPortal(EPortalType _PortalType, FHitResult _HitResult);

	bool GetCloneAlive() { return m_bCloneAlive; }

	USkeletalMeshComponent* GetCharacterMesh() { return m_CharacterMesh;}
	USkeletalMeshComponent* GetVisualGunMesh() { return m_VisualGun; }
	USkeletalMeshComponent* GetArmsMesh() { return m_Arms; }
	USkeletalMeshComponent* GetGunMesh() { return m_Gun; }
	bool GetDotResult() { return m_bDotResult; }

	
	
};

