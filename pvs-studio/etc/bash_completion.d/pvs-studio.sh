_args() {
    local cur prev program comparg
    if type -t _init_completion >/dev/null; then
        _init_completion -n = 2> /dev/null
        _get_comp_words_by_ref cur prev 2> /dev/null
    else
        COMPREPLY=()
        cur="${COMP_WORDS[COMP_CWORD]}"
        prev="${COMP_WORDS[COMP_CWORD-1]}"
    fi

    program="${COMP_WORDS[0]}"
    comparg="--complete"

    COMPREPLY=($("$program" "$comparg" bash "$COMP_CWORD" "${COMP_WORDS[@]}" 2> /dev/null))
    [[ $COMPREPLY ]] && return
    _filedir
}

complete -F _args plog-converter pvs-studio-analyzer
