node('master') {
    echo 'checking out met-api.git in workspace on master ...';
    git url: 'https://github.com/metno/met-api.git';
    echo 'done';

    //ws = sh 'which sleep';
    def ws = 'foo';
    echo "${ws}";
    echo "sleepHome: ${ws}";

}
