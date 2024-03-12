void cmdInterpreter(std::vector<std::string> &cmd);

enum command_code {
    eHelp,
    eSetWifi,
    eMnemonic,
    eStoreSeed,
    eGetPublicKey,
    eAttestMachine,
    eNotarizeData,
    eCreateCsr,
    eStoreCert,
    eReadCert,
    eTestAWSMQTTMsg,
    eBalance,
    NumOfUserCommand
};