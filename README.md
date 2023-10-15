# Computing Conjuction Detection Messages (CDM) from Satellite Data and storing them on a block-chain (DEMO)

This repository contains a demonstration of CDM generation from [space-track.org]() data inside a mining node and its subsequent storage on block chain.

## How to build (Dockerfile):

The docker file is the easiest way to build the code. Clone the repository and from with in the directory run:

```bash
docker build -t trckdspace .
```

This will pull all the dependencies and build the executable.

## Running the demo inside the container

To execute the demo, we first need to launch the container in interactive mode

```bash
docker run -it trckdspace
```

and inside the container, run the executable as follows

```bash
cd cdm_on_chain/build
./run_chain
```

This will start the miner that includes a conjunction detection algorithm. The output will be printed to the console as data is generated and added to block (This takes time).
