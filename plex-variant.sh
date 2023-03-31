set -e

BLOB_PATH=$1
KEYSET_PATH=$2
DB_PATH=$3
ACTION=$4
RELOAD_FILE=$5
if [ "$#" -ne 5 ]
then
  echo "Require 5 argument (BLOB_PATH, KEYSET_PATH, DB_PATH, ACTION, RELOAD_FILE), $# provided"
  exit 1
fi

if [[ $ACTION != "build" && $ACTION != "benchmark" ]]
then
  echo "Invalid ACTION [build | benchmark]"
  exit 1
fi
echo "Exploring PLEX's hyperparameter with ACTION= ${ACTION}"

DATASET_NAME="books_800M_uint64"
NUM_KEYS="800000000"
OUT_PATH="out"
mkdir -p ${OUT_PATH}

ERRORS=(
  "65536"
  "32768"
  "16384"
  "8192"
  "4096"
  "2048"
  "1024"
  "512"
  "256"
  # "128"
  # "64"
  # "32"
  # "16"
  # "8"
  # "4"
  # "2"
  # "1"
)

mkdir -p ${DB_PATH}

echo "Start Building PLEX"

build () {
  error=$1

  set -x
  ./build/kv_build \
      --keys_file=${BLOB_PATH}/${DATASET_NAME} \
      --keys_file_type=sosd \
      --total_num_keys=${NUM_KEYS} \
      --db_path=${DB_PATH}/${DATASET_NAME}_e${error} \
      --max_error=${error}
  set +x
}

benchmark () {
  error=$1

  for ((j = 0; j < 40; j++)) do
    echo ">>> ${DATASET_NAME}, error= ${error}, keyset= ${j}"
    bash ${RELOAD_FILE}
    set -x
    ./build/kv_benchmark \
        # --num_samples=10000 \
        --key_path=${KEYSET_PATH}/${DATASET_NAME}_ks_${j} \
        --target_db_path=${DB_PATH}/${DATASET_NAME}_e${error} \
        --out_path=${OUT_PATH}/${DATASET_NAME}_e${error}_out.txt
    set +x
  done
}

for ((i = 0; i < ${#ERRORS[@]}; i++)) do
    error=${ERRORS[$i]}
    echo ">>> ${DATASET_NAME}, error= ${error}"
    if [[ $ACTION == "build" ]]
    then
      build ${error}
    elif [[ $ACTION == "benchmark" ]]
    then
      benchmark ${error}
    fi
done
