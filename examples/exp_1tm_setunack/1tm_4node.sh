c#! /bin/sh -x
#
# manage_exp.sh
# Copyright (C) 2018 Cenk Gündoğan <cenk.guendogan@haw-hamburg.de>
# Copyright (C) 2019 Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
#
# Distributed under terms of the MIT license.
#

EXPNAME=4nodes_1tm_1-100

SCRIPTBASE="$(dirname $(realpath $0))"

IOTLAB_USER="${IOTLAB_USER:-$(cut -f1 -d: ${HOME}/.iotlabrc)}"
IOTLAB_SITE="${IOTLAB_SITE:-saclay}"

# Duration in minutes
IOTLAB_DURATION=${IOTLAB_DURATION:-200}

SACLAY_NODES="1-4"
NUM_NODES=${NUM_NODES:-4}

REQUESTS=${REQUESTS:-100}

# average request rate per node / node in the fib
DELAY_REQUEST=${DELAY_REQUEST:-1000000} # in us
DELAY_JITTER=${DELAY_JITTER:-500000} # in us
TIMEOUT=${TIMEOUT:-151}


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
sleep 5
tmux send-keys -t riot-${EXPID}:2 "reboot" C-m
sleep 5
# tmux send-keys -t riot-${EXPID}:2 ""
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-1;cfg_source" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-2;cfg_sink" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-3;cfg_sink" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-4;cfg_sink" C-m

# Probe for background traffic
tmux send-keys -t riot-${EXPID}:2 "clr" C-m
sleep 10
tmux send-keys -t riot-${EXPID}:2 "stats" C-m
sleep 1

# Run expiriment
tmux send-keys -t riot-${EXPID}:2 "clr" C-m
sleep 1
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-1;run_lvl ${REQUESTS} ${DELAY_REQUEST}" C-m
sleep ${TIMEOUT}
tmux send-keys -t riot-${EXPID}:2 "stats" C-m
sleep 1

# Probe for background traffic
tmux send-keys -t riot-${EXPID}:2 "clr" C-m
sleep 10
tmux send-keys -t riot-${EXPID}:2 "stats" C-m
sleep 1

# Cleanup
iotlab-experiment stop -i ${EXPID} > /dev/null
CMD
)

ssh ${IOTLAB_USER}@${IOTLAB_SITE}.iot-lab.info -t << EOF
tmux new-session -d -s riot-${EXPID} "${CMDS1}"
tmux new-window -t riot-${EXPID}:2 "${CMDS2}"
${EXPCMDS}
tmux kill-session -t riot-${EXPID}
EOF

exit 0
