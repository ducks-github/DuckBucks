#ifndef MIMBLEWIMBLE_INIT_H
#define MIMBLEWIMBLE_INIT_H

#include "init.h"
#include "wallet.h"
#include "mimblewimble.h"
#include "mimblewimble_wallet.h"

/** Initialize Mimblewimble protocol for DuckBucks */
bool InitMimblewimbleProtocol();

/** Register wallet hooks for Mimblewimble functionality */
bool RegisterMimblewimbleWalletHooks();

/** Save Mimblewimble wallet data */
bool SaveMimblewimbleData();

/** Run a test transaction using Mimblewimble */
void RunMimblewimbleTest();

#endif // MIMBLEWIMBLE_INIT_H 