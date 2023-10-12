// ====================================================
// EasyBlockChain Copyright(C) 2018 Furkan T�rkal
// This program comes with ABSOLUTELY NO WARRANTY; This is free software,
// and you are welcome to redistribute it under certain conditions; See
// file LICENSE, which is part of this source code package, for details.
// ====================================================

#pragma once

#include <cstdint>
#include <iostream>

#include "CDM.hpp"
#include "propagator/SimulatorSGP4.hpp"

using namespace std;

class CBlock final
{

public:
	string PrevHash;

	CBlock(std::uint32_t indexIn);

	void SetAsGenesis();

	uint32_t GetIndex() const;

	time_t GetTime() const;

	string GetHash() const;

	void DOMine(uint32_t difficulty, SimulatorSGP4 *, double);

	std::vector<CDM> GetData() const;

private:
	uint32_t m_index;
	int64_t m_nonce;
	std::vector<CDM> m_data;
	string m_hash;
	time_t m_time;

	const string CalculateHash() const;
};