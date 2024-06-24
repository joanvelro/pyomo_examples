# Pyomo Examples

## Create virtual env

* Create virtual env
```
cd <project_path>
python -m virtualenv ./env/ --python=<path_to_python_exe_version>
```

``` linux/mac
python -m virtualenv ./.env/ --python=/usr/local/bin/python3.11
```

``` windows
python -m virtualenv ./.env/ --python=C:\Python311\python.exe
```

* Activate virtual env

``` windows cmd
./env/Scripts/activate.bat (cmd windows)
```
```powershell window
./env/Scripts/Activate.ps1 (powershell windows)
```
``` linux/mac
source ./.env/bin/activate (linux/Mac)
```

* install poetry in your python environment through.

```
pip install poetry==1.6
```

* install the poetry from the _pyproject.toml_ file or from a _poetry.lock_ file (if exist)

```
poetry install --no-interaction --no-ansi --no-root 
```


Options:

* --without docs : to not install docs group dependencies
* --only docs : only install docs group dependencies
* --with dev: optional dependency group dev included
* --no-root: not install root package (library artifact)
* --no-interaction: no interaction with the installation
* --no-ansi: disable ANSI output


* add the environment to the jupyter notebook
```
pip install ipykernel
```
```
python -m ipykernel install --user --name=<env_name>
```

* Update the environment to the latest version of the dependencies and to update the poetry.lock
file:

```
poetry update
```