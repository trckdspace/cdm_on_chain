// ====================================================
// EasyBlockChain Copyright(C) 2018 Furkan T�rkal
// This program comes with ABSOLUTELY NO WARRANTY; This is free software,
// and you are welcome to redistribute it under certain conditions; See
// file LICENSE, which is part of this source code package, for details.
// ====================================================

#include "BlockChain.h"

int main(int argc, char *argv[])
{
	CBlockChain bChain = CBlockChain("../data/3le.txt", 2);

	bChain.SetPropagationResolutionSeconds(0.01);

	cout << "Genesis..." << endl;
	bChain.AddGenesis();

	int count = 1;
	while (1)
	{
		cout << "Mining block " << count << endl;
		bChain.AddBlock(CBlock(count));
		count++;
	}

	return 0;
}