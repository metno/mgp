node('master') {
    echo 'checking out met-api.git in workspace on master ...';
    git url: 'https://github.com/metno/met-api.git';
    echo 'done';

    sh 'which sleep';
    echo 'sleepHome:' $sleepHome;

}
