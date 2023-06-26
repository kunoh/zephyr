#!/bin/bash
# [fea|bug: [critical]|ref]: [feature]: [description]
# Merged PR 30470: fea: pipeline: Add pr title verification pipeline
echo "PR title is: $1"
read -ra arr <<<"$1"
allowed_pr_types=('"ref:' '"bug:' '"fea:')
valid=true
re='^[0-9]+([:]+)$'

# Verify "Merged" case
if [ "${arr[0]}" == '"Merged' ]; then
  ((offset=3))
  if [[ ! ${arr[1]} =~ '"PR' ]]; then
    valid=false
    echo "[Error] Word 'PR' should follow after Merged"
  elif ! [[ ${arr[2]} =~ $re ]] ; then
    valid=false
    echo "[Error] Word 'PR' should follow after Merged"
  fi

else
  ((offset=0))
fi

# Both cases
if [ ${#arr[@]} -lt $((3+offset)) ]; then
  valid=false
  echo "[Error] PR title too short. Should follow the pattern: [fea|bug: [critical]|ref]: [feature]: [description]"
elif [[ ! " ${allowed_pr_types[*]} " =~ ${arr[0+offset]} ]]; then
  valid=false
  echo "[Error] First word in PR title should be one of: [fea|bug: [critical]|ref]"
elif [ "${arr[1+offset]}" == "critical:" ]; then
  if [[ "${arr[2+offset]}" != *: ]]; then
    valid=false
    echo "[Error] PR title should follow the pattern: [fea|bug: [critical]|ref]: [feature]: [description]"
  fi
  if [ ${#arr[@]} -lt $((4+offset)) ]; then
    valid=false
    echo "[Error] PR title misses description"
  fi
elif [[ "${arr[1+offset]}" != *: ]]; then
  valid=false
  echo "[Error] PR title should follow the pattern: [fea|bug: [critical]|ref]: [feature]: [description]"
fi

$valid && exit 0
exit 1