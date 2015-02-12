node('master') {
    echo 'checking out met-api.git in workspace on master ...';
    git url: 'https://github.com/metno/met-api.git';
    echo 'done';

    def sleepHome = tool 'sleep';
    echo 'sleepHome:' $sleepHome;

}
