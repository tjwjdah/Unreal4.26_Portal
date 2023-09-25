// Fill out your copyright notice in the Description page of Project Settings.


#include "Crosshair.h"

void UCrosshair::NativeConstruct()
{
	Super::NativeConstruct();

	m_CrosshairImage = Cast<UImage>(GetWidgetFromName(TEXT("CrosshairImage")));
	m_PanelSlot = Cast<UCanvasPanelSlot>(m_CrosshairImage->Slot);
	UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
	/*
	if (ViewportClient)
	{
		FViewport* Viewport = ViewportClient->Viewport;
		HUDSize = Viewport->GetSizeXY();
		//HUDSize /= 2;
		HUDSize.X -= 40;
		HUDSize.Y -= 5;
		m_PanelSlot->SetPosition(HUDSize);
	}
	*/
}
void UCrosshair::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

}



