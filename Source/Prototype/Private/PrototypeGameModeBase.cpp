// Copyright 2019

#include "PrototypeGameModeBase.h"
#include "PrototypeGameInstance.h"

void APrototypeGameModeBase::AddToJewelsAndScore(const int32 jewels)
{
	Jewels += jewels;
	
	// Score takes into account the number of jewels collected in one go
	Score += jewels * (jewels - 2);
}
