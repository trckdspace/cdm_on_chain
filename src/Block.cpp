// ====================================================
// EasyBlockChain Copyright(C) 2018 Furkan T�rkal
// This program comes with ABSOLUTELY NO WARRANTY; This is free software,
// and you are welcome to redistribute it under certain conditions; See
// file LICENSE, which is part of this source code package, for details.
// ====================================================

#include <ctime>
#include <sstream>
#include "Block.hpp"
#include "SHA256.h"

CBlock::CBlock(uint32_t indexIn) : m_index(indexIn)
{
	this->m_nonce = -1;
	this->m_time = time(nullptr);
}

uint32_t CBlock::GetIndex() const
{
	return this->m_index;
}

time_t CBlock::GetTime() const
{
	return this->m_time;
}

string CBlock::GetHash() const
{
	return this->m_hash;
}

void CBlock::SetAsGenesis()
{
	this->m_nonce = -1;
	this->m_time = time(nullptr);
	this->m_index = 0;
	// this->m_data = "";
	this->m_hash = "Genesis";
}

void CBlock::DOMine(uint32_t difficulty, SimulatorSGP4 *propagator, double resolution)
{

	while (this->m_data.size() < difficulty)
	{
		propagator->propagate(resolution, this->m_data);
	}

	char *cstr = new char[difficulty + 1];

	for (uint32_t i = 0; i < difficulty; ++i)
	{
		cstr[i] = '0';
	}
	cstr[difficulty] = '\0';

	string str(cstr);

	do
	{
		this->m_nonce++;
		this->m_hash = CalculateHash();
	} while (!str.compare(this->m_hash.substr(0, difficulty)));

	cout << "Block mined: " << this->m_hash << endl;
	cout << "CDMs recorded " << std::endl;
	for (auto msg : this->m_data)
	{
		cout << "-------" << std::endl;
		cout << "TCA" << msg.TimeClosestApproach << std::endl
			 << "Sat1 NORADID " << msg.sat1_satnumber << std::endl
			 << "Sat2 NORADID " << msg.sat2_satnumber << std::endl
			 << "Distance (km) " << msg.min_distance << std::endl
			 << "Relative Velocity (km) " << msg.relative_velocity << std::endl;
		;
	}
}

std::vector<CDM> CBlock::GetData() const
{
	return m_data;
}

inline const string CBlock::CalculateHash() const
{
	stringstream ss;
	ss << this->m_index << this->m_time;

	for (auto msg : this->m_data)
	{
		ss << "-------" << std::endl;
		ss << "TCA" << msg.TimeClosestApproach << std::endl
		   << "Sat1 NORADID " << msg.sat1_satnumber << std::endl
		   << "Sat2 NORADID " << msg.sat2_satnumber << std::endl
		   << "Distance (km) " << msg.min_distance << std::endl
		   << "Relative Velocity (km) " << msg.relative_velocity << std::endl;
		;
	}

	ss << this->m_nonce << this->PrevHash;
	return sha256(ss.str());
}