# setup script for HcNano package within CMSSW

# git settings
BRANCH=main
GIT_USER=LukaLambrecht
REPO_NAME=HcNano

# other settings
RELEASE=CMSSW_13_0_10
# (note: maybe make command line arg?)

# install CMSSW
echo "Installing $RELEASE..."
scram project CMSSW $RELEASE
cd $RELEASE/src
eval `scram runtime -sh`
echo "Current CMSSW_BASE: $CMSSW_BASE"

# git clone this repository
echo "Cloning the $REPO_NAME repository..."
git cms-init
git clone https://github.com/$GIT_USER/$REPO_NAME PhysicsTools/$REPO_NAME

# compile and move into package
echo "Compiling..."
cd $CMSSW_BASE/src/PhysicsTools/$REPO_NAME
scramv1 b
echo "Setup finished"
