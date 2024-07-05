import os
import shutil


def check_environment():
    """
    *Check Environment*

    Check if the python environment is correctly configured
    """
    if not shutil.which("pyomo"):
        os.system('pip install -q pyomo')
        assert (shutil.which("pyomo"))
    if not shutil.which("ortools"):
        os.system('pip install ortools')
    if not (shutil.which("cbc") or os.path.isfile("cbc")):
        if "google.colab" in sys.modules:
            os.system('apt-get install -y -qq coinor-cbc')
        else:
            try:
                os.system('conda install -c conda-forge coincbc')
            except Exception as err:
                logger.info('Error in Check Environment{}'.format(err))
                pass

    assert (shutil.which("cbc") or os.path.isfile("cbc"))
