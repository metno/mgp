SELECT networkid, userid, networkname, identityid, encodingcodec, decodingcodec, servercodec,
       userandomserver, perform, useautoidentify, autoidentifyservice, autoidentifypassword,
       useautoreconnect, autoreconnectinterval, autoreconnectretries, unlimitedconnectretries,
       rejoinchannels, connected, usermode, awaymessage, attachperform, detachperform,
       usesasl, saslaccount, saslpassword
FROM network
