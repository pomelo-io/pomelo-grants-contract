#!/bin/bash
echo "Killing nodeos ..."
./scripts/kill_nodeos.sh

echo "Starting nodeos ..."
./scripts/start_nodeos.sh

sleep 5
echo "Deploying contracts ..."
./scripts/deploy.sh
