#! /bin/sh -x
#
# manage_exp.sh
# Copyright (C) 2018 Cenk Gündoğan <cenk.guendogan@haw-hamburg.de>
# Copyright (C) 2019 Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
#
# Distributed under terms of the MIT license.
#

EXPNAME=many_to_1_setunack_5s_100iter

SCRIPTBASE="$(dirname $(realpath $0))"

IOTLAB_USER="${IOTLAB_USER:-$(cut -f1 -d: ${HOME}/.iotlabrc)}"
IOTLAB_SITE="${IOTLAB_SITE:-saclay}"

# Duration in minutes
IOTLAB_DURATION=${IOTLAB_DURATION:-200}

SACLAY_NODES="1-10"

REQUESTS=${REQUESTS:-100}
DELAY_REQUEST=${DELAY_REQUEST:-5000000} # in us

# build the application
binroot="../exp_1tm_setunack"

make -C ${binroot} -B clean all || {
    echo "building firmware failed!"
    exit 1
}

echo "Time until ps(): $(((($REQUESTS*$DELAY_REQUEST)/1000000)+10))"


# submit experiment
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

EXPCMDS=$(cat << CMD
tmux send-keys -t riot-${EXPID}:2 "reboot" C-m
sleep 5
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-1;cfg_sink" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-2;cfg_source" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-3;cfg_source" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-4;cfg_source" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-5;cfg_source" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-6;cfg_source" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-7;cfg_source" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-8;cfg_source" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-9;cfg_source" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-10;cfg_source" C-m
tmux send-keys -t riot-${EXPID}:2 "clr" C-m
sleep 3
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-2;run_lvl ${REQUESTS} ${DELAY_REQUEST}" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-3;run_lvl ${REQUESTS} ${DELAY_REQUEST}" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-4;run_lvl ${REQUESTS} ${DELAY_REQUEST}" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-5;run_lvl ${REQUESTS} ${DELAY_REQUEST}" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-6;run_lvl ${REQUESTS} ${DELAY_REQUEST}" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-7;run_lvl ${REQUESTS} ${DELAY_REQUEST}" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-8;run_lvl ${REQUESTS} ${DELAY_REQUEST}" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-9;run_lvl ${REQUESTS} ${DELAY_REQUEST}" C-m
tmux send-keys -t riot-${EXPID}:2 "nrf52dk-10;run_lvl ${REQUESTS} ${DELAY_REQUEST}" C-m
# sleep $(((($REQUESTS*$DELAY_REQUEST)/1000000)+10))
sleep 760
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
