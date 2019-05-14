#! /bin/sh -x
#
# manage_exp.sh
# Copyright (C) 2018 Cenk Gündoğan <cenk.guendogan@haw-hamburg.de>
# Copyright (C) 2019 Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
#
# Distributed under terms of the MIT license.
#

EXPNAME=1_to_many_setunack_mhop_1s_100iter

SCRIPTBASE="$(dirname $(realpath $0))"

IOTLAB_USER="${IOTLAB_USER:-$(cut -f1 -d: ${HOME}/.iotlabrc)}"
IOTLAB_SITE="${IOTLAB_SITE:-saclay}"

# Duration in minutes
IOTLAB_DURATION=${IOTLAB_DURATION:-200}

# sniffer node should node be part of experiment nodes lsit
SNIFFER="12"

SACLAY_NODES="1-10"
NUM_NODES=${NUM_NODES:-10}

# node roles
PRODUCER="11"
# CONSUMER="1"
SOURCE="${SOURCE:-1}"

# buidl configurations
DEFAULT_CHANNEL=17

REQUESTS=${REQUESTS:-100}

# average request rate per node / node in the fib
DELAY_REQUEST=${DELAY_REQUEST:-1000000} # in us
DELAY_JITTER=${DELAY_JITTER:-500000} # in us


FLAGS=
# FLAGS="-DIEEE802154_DEFAULT_CHANNEL=${DEFAULT_CHANNEL} \
#        -DNUM_REQUESTS_NODE=${REQUESTS}"
        # -Wno-error=cast-function-type"

# USEMODULES to build in RIOT
UMODS=""

if [ ! -z "${DELAY_REQUEST}" ]; then
    FLAGS="${FLAGS} -DDELAY_REQUEST=${DELAY_REQUEST}"
fi
if [ ! -z "${DELAY_JITTER}" ]; then
    FLAGS="${FLAGS} -DDELAY_JITTER=${DELAY_JITTER}"
fi

# build the application
binroot="../exp_1tm_setunack"

#CFLAGS="${FLAGS}" USEMODULE+="${UMODS}" make -C ${binroot} clean all || {
make -C ${binroot} -B clean all || {
    echo "building firmware failed!"
    exit 1
}

echo "Time until ps(): $(((($REQUESTS*$DELAY_REQUEST)/1000000)+10))"


# submit experiment
#EXPID=$(iotlab-experiment submit -n ${EXPNAME} -d $((IOTLAB_DURATION + 3)) -l ${IOTLAB_SITE},m3,${IOTLAB_NODES},${binroot}/bin/iotlab-m3/${binroot##*/}.elf | grep -Po '[[:digit:]]+')
EXPID=$(iotlab-experiment submit -n ${EXPNAME} -d $((IOTLAB_DURATION + 3)) -l ${IOTLAB_SITE},nrf52dk,${SACLAY_NODES},${binroot}/bin/nrf52dk/${binroot##*/}.elf | grep -Po '[[:digit:]]+')

if [ -z "${EXPID}" ]; then
    echo "experiment submission failed!"
    exit 1
fi


iotlab-experiment wait -i ${EXPID} || {
    echo "experiment startup failed!"
    exit 1
}

NUM_EXP_NODES=$(iotlab-experiment get -r | grep archi | wc -l)
SUBMISSION_TIME=$(date +%d-%m-%Y"-"%H-%M)
NAME="$EXPNAME-$EXPID-$IOTLAB_SITE-$NUM_EXP_NODES-$REQUESTS-$SUBMISSION_TIME"

echo "Experiment name is: ${NAME}"

CMDS1=$(cat << CMD
sleep $((IOTLAB_DURATION * 60))
iotlab-experiment stop -i ${EXPID} > /dev/null
CMD
)
CMDS2=$(cat << CMD
serial_aggregator -i ${EXPID} | tee ${NAME}.log
CMD
)
# tmux send-keys -t riot-${EXPID}:2 "pktcnt_p" C-m
# sniffer_aggregator -i ${EXPID} -l ${IOTLAB_SITE},n,${SNIFFER} -o ${NAME}.pcap &

EXPCMDS=$(cat << CMD
tmux send-keys -t riot-${EXPID}:2 "reboot" C-m
sleep 5
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-1;wl BC7F3FBA3EE7" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-2;wl 06674DBF94F2" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-2;wl BDD2D5D260EE" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-3;wl BC7F3FBA3EE7" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-3;wl A0DE2BCAC3CC" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-4;wl BDD2D5D260EE" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-4;wl CD118380D4C1" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-5;wl A0DE2BCAC3CC" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-5;wl 46BD07D43BEF" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-6;wl CD118380D4C1" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-6;wl 1D5C150F79E1" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-7;wl 46BD07D43BEF" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-7;wl 8E7C7B5DB5F6" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-8;wl 1D5C150F79E1" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-8;wl 4F5DE589B3E1" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-9;wl 8E7C7B5DB5F6" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-9;wl 7CF73D6790CD" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-10;wl 4F5DE589B3E1" C-m

tmux send-keys -t riot-${EXPID}:2 "nrf52dk-1;cfg_source" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-2;cfg_sink" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-3;cfg_sink" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-4;cfg_sink" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-5;cfg_sink" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-6;cfg_sink" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-7;cfg_sink" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-8;cfg_sink" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-9;cfg_sink" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-10;cfg_sink" C-m

tmux send-keys -t riot-${EXPID}:2 "clr" C-m
sleep 3
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-1;run_lvl ${REQUESTS} ${DELAY_REQUEST}" C-m
# sleep $(((($REQUESTS*$DELAY_REQUEST)/1000000)+10))
sleep 200
tmux send-keys -t riot-${EXPID}:2 "stats" C-m
sleep 5
iotlab-experiment stop -i ${EXPID} > /dev/null
CMD
)

ssh ${IOTLAB_USER}@${IOTLAB_SITE}.iot-lab.info -t << EOF
tmux new-session -d -s riot-${EXPID} "${CMDS1}"
tmux new-window -t riot-${EXPID}:2 "${CMDS2}"
iotlab-node -i ${EXPID} --reset
${EXPCMDS}
EOF

exit 0
