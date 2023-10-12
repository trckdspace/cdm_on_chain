// ====================================================
// EasyBlockChain Copyright(C) 2018 Furkan T�rkal
// This program comes with ABSOLUTELY NO WARRANTY; This is free software,
// and you are welcome to redistribute it under certain conditions; See
// file LICENSE, which is part of this source code package, for details.
// ====================================================

#pragma once

#include <cstdint>
#include <vector>
#include "Block.hpp"

using namespace std;

class CBlockChain
{
public:
	CBlockChain(const char *, int);

	void AddGenesis();

	void AddBlock(CBlock newBlock);

	void PrintBlocks();

	void SetPropagationResolutionSeconds(double frequency);

private:
	uint32_t m_difficulty;
	vector<CBlock> m_chain;

	const CBlock GetLastBlock() const;

	const uint32_t GetChainSize() const;

	SimulatorSGP4 *propagator;

	double m_propagationResolution;

	// const uint32_t GetChainCapacity() const;
};