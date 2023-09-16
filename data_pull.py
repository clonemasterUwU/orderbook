import os
import gzip
import sys
import urllib.request
from multiprocessing import Pool, cpu_count
from functools import partial
import shutil


def download_zip(url, data_path):
    try:
        file_name = url.split("/")[-1]
        extracted_filepath = os.path.join(data_path, os.path.splitext(file_name)[0])  # Remove the .gz extension
        print("Download and gunzip file {}:".format(extracted_filepath))
        with urllib.request.urlopen(url) as response:
            with gzip.GzipFile(fileobj=response) as uncompressed:
                with open(extracted_filepath, 'wb') as extracted_file:
                    shutil.copyfileobj(uncompressed, extracted_file)
    except Exception as e:
        print(e)


if __name__ == "__main__":
    file_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")
    print("Download to {}".format(file_path))
    urls = ['https://emi.nasdaq.com/ITCH/Nasdaq%20BX%20ITCH/20181228.BX_ITCH_50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20BX%20ITCH/20190130.BX_ITCH_50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20BX%20ITCH/20190327.BX_ITCH_50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20BX%20ITCH/20190530.BX_ITCH_50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20BX%20ITCH/20190730.BX_ITCH_50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20BX%20ITCH/20190830.BX_ITCH_50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20BX%20ITCH/20191030.BX_ITCH_50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20BX%20ITCH/20191230.BX_ITCH_50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20BX%20ITCH/20200130.BX_ITCH_50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/01302019.NASDAQ_ITCH50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/01302020.NASDAQ_ITCH50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/03272019.NASDAQ_ITCH50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/07302019.NASDAQ_ITCH50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/08302019.NASDAQ_ITCH50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/10302019.NASDAQ_ITCH50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/12282018.NASDAQ_ITCH50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/12302019.NASDAQ_ITCH50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20PSX%20ITCH/05302019.NASDAQ_ITCH50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20PSX%20ITCH/20181228.PSX_ITCH_50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20PSX%20ITCH/20190130.PSX_ITCH_50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20PSX%20ITCH/20190327.PSX_ITCH_50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20PSX%20ITCH/20190530.PSX_ITCH_50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20PSX%20ITCH/20190730.PSX_ITCH_50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20PSX%20ITCH/20190830.PSX_ITCH_50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20PSX%20ITCH/20191030.PSX_ITCH_50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20PSX%20ITCH/20191230.PSX_ITCH_50.gz',
            'https://emi.nasdaq.com/ITCH/Nasdaq%20PSX%20ITCH/20200130.PSX_ITCH_50.gz']

    if len(sys.argv) > 1:
        urls = urls[:int(sys.argv[1])]
    print("There are {} CPUs on this machine ".format(cpu_count()))
    pool = Pool(cpu_count())
    download_func = partial(download_zip, file_path=file_path)
    results = pool.map(download_func, urls)
    pool.close()
    pool.join()
