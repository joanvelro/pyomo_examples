import pyodbc
import time
import logging

from configparser import ConfigParser
from src.utils import initialize_engine

# Start time
start = time.time()

""" Set logging file
"""
logger = logging.getLogger('reports\\logs\\log')
logger.setLevel(logging.DEBUG)
fh = logging.FileHandler('reports\\logs\\log.log')
fh.setLevel(logging.DEBUG)
logger.addHandler(fh)
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
fh.setFormatter(formatter)
logger.addHandler(fh)
logging.getLogger().addHandler(logging.StreamHandler())  # to display in console message
# logger.debug('mensaje debug')
# logger.info('mensaje info')
# logger.warning('mensaje warning')
# logger.error('mensaje error')
# logger.critical('mensaje critical')


# Read config.ini file
config_object = ConfigParser()
config_object.read("config.ini")
print(config_object._sections)

# get paths from config file
data_path = config_object._sections["paths"]['data_path']
results_table = config_object._sections["database_tables"]['results_table']

db_ser = True
db_pre = False

if db_pre:  # postgreSQL
    db_settings = config_object._sections["DB"]
    engine = initialize_engine(db_settings)
if db_ser:  # SQLserver
    db_settings = config_object._sections["DB_SER"]

driver = db_settings['driver']
server = db_settings['server']
database = db_settings['database']
user = db_settings['user']
password = db_settings['pass']
schema = db_settings['schema']
port = db_settings['port']

df_results = []

""" Query to execute postgresql
"""
if db_pre:
    logger.info('{} Server:[{}] database:[{}] schema:[{}] '.format('-' * 20, server, database, schema))

    try:
        df_results.to_sql(name=results_table,
                          con=engine,
                          schema=schema,
                          if_exists='append',  # replace
                          chunksize=10000,
                          index=False)
    except Exception as exception_msg:
        logger.error(
            '{} (!) Error in save results in database postgreSQL: '.format('-' * 20) + str(exception_msg))
        error = 1

""" Query to execute SQLserver
"""
if db_ser:
    logger.info('Server:[{}] database:[{}] schema:[{}] '.format(server, database, schema))
    sql_conn_str = 'DRIVER={};SERVER={},{};DATABASE={};UID={};PWD={}'.format(driver, server, port, database, user,
                                                                             password)

    try:
        sql_conn = pyodbc.connect(sql_conn_str)
        cursor = sql_conn.cursor()
        for index, row in df_results.iterrows():
            # print(row)
            cursor.execute(
                "INSERT INTO H_PREDICTIVE_SECTION("
                "   [logic_code],"
                "   [Fecha],"
                "   [Section_Alias],"
                "   [Prediccion_int_15],"
                "   [Prediccion_int_60],"
                "   [Prediccion_int_120],"
                "   [Ind_accidente_15],"
                "   [Ind_accidente_60],"
                "   [Ind_accidente_120]) "
                "values(?,?,?,?,?,?,?,?,?)",
                row['logic_code'],
                row['Fecha'],
                row['Section_Alias'],
                row['Prediccion_int_15'],
                row['Prediccion_int_60'],
                row['Prediccion_int_120'],
                row['Ind_accidente_15'],
                row['Ind_accidente_60'],
                row['Ind_accidente_120'])

        sql_conn.commit()
        cursor.close()
        sql_conn.close()
    except Exception as exception_msg:
        logger.error(
            '{} (!) Error in save results in database sqlserver: '.format('-' * 20) + str(exception_msg))
        error = 1
