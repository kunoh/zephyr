
import datetime
import argparse
import io
import re
import pandas as pd

from jinja2 import Environment, FileSystemLoader
from atlassian import Confluence
from bs4 import BeautifulSoup

SPACE = 'HWEMBSW'
ROOT_PAGE = 'Releases - MCU'

RELEASE_PATTERN = r'v[0-9]+\.[0-9]+\.[0-9]+'
BETA_PATTERN = r'v[0-9]+\.[0-9]+\.[0-9]+-beta.[0-9]+'
DEV_PATTERN = r'v[0-9]+\.[0-9]+\.[0-9]+-(?!beta).+[0-9]+'

product_friendly_names = {
    "tma0005": "TM5 - TMA0005"
}

target_friendly_names = {
    "io": "IO App",
    "io-boot": "IO Bootloader",
    "io1060": "IO1060 App",
    "io1060-boot": "IO1060 Bootloader",
    "ble": "BLE App",
    "ble-boot": "BLE Bootloader"
}

class BuildInfo:
    def __init__(self, target, version, url, product, changes):
        self.target = target
        self.version = version
        self.url = url
        self.product = product
        self.changes = changes
        self.date = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")


def open_connection_to_confluence(username, password):
    '''Open a connection to confluence'''
    return Confluence(
        url='https://trackman.atlassian.net',
        username=username,
        password=password)

def create_child_page(cf, target, parent_id):
    '''Create a new child page if it does not already exists'''
    if not cf.page_exists(SPACE, target):
        print('Creating page')
        cf.create_page(SPACE, target, '', parent_id=parent_id)
    else:
        print('Page already exists')

def parse_page_body(page):
    '''Parse html from page body'''
    return page['body']['storage']['value']

def table_html_to_dataframe(html, columns):
    '''Load table into dataframe if exist else create a new table'''
    try:
        df_table = pd.read_html(html)
        df = df_table[0]
    except Exception as e:
        print("Could not load html, creating a new table")
        print(e)
        df = pd.DataFrame(columns=columns)
    return df

def fix_values_from_conversion(df, dict_of_values):
    '''A hack to keep hyperlink in table, as Pandas only read html as text'''
    if df['Known Issues'].dtypes == 'float64':
        df['Known Issues'] = df['Known Issues'].astype('string')
    if df['Changes'].dtypes == 'float64':
        df['Changes'] = df['Changes'].astype('string')
    for i, (version, known_issue, changes) in enumerate(zip(dict_of_values['version_list'], dict_of_values['known_issues_list'], dict_of_values['changes_list'])):
        df.at[i, 'Version'] = version
        df.at[i, 'Known Issues'] = known_issue
        df.at[i, 'Changes'] = changes
    return df

def fix_hyperlink_from_conversion(df, column, links):
    '''A hack to keep hyperlink in table, as Pandas only read html as text'''
    for i, link in enumerate(links):
        df.at[i, column] = link
    return df

def parse_href_from_table(html_text):
    soup = BeautifulSoup(html_text, 'html.parser')
    table = soup.find('table')

    links = []
    if table:
        for tr in table.findAll("tr"):
            trs = tr.findAll("td")
            for each in trs:
                try:
                    link = each.find('a')['href']
                    text = each.text
                    links.append(f'<a href="{link}">{text}</a>')
                except:
                    pass
    return links

def parse_cells_from_table(html_text):
    soup = BeautifulSoup(html_text, 'html.parser')
    table = soup.find('table')

    table_content = {
        'version_list': [],
        'known_issues_list': [],
        'changes_list': []
    }
    if table:
        for tr in table.findAll("tr"):
            column_counter = 0
            trs = tr.findAll("td")
            for each in trs:
                if column_counter == 0:
                    column_counter += 1
                    continue
                if column_counter == 1:
                    table_content['version_list'].append(each.decode_contents())
                elif column_counter == 2:
                    table_content['known_issues_list'].append(each.decode_contents())
                elif column_counter == 3:
                    table_content['changes_list'].append(each.decode_contents())
                column_counter += 1

    return table_content

def append_data_to_dataframe(df, data_list):
    '''Create a dataframe with data and append to an existing one'''
    df_new_data = pd.DataFrame([data_list], columns=df.columns)
    return df.append(df_new_data).reset_index(drop=True)

def update_data_in_dataframe(bi, df):
    if bi.product:
        row_idx = df[df['Variant']==bi.product].index.values[0]
    else:
        row_idx = df[df['Model']==bi.target].index.values[0]
    df.at[row_idx,'Version'] = f'<a href="{bi.url}">{bi.version}</a>'
    return df

def add_color_to_cells(lines, replacement, number_of_colums, color=None, start_index=0, stop_index=0):
    for i, line in enumerate(lines[start_index:], start=start_index):
        if color and i < stop_index:
            changes = line.replace('<td>', f'<td class="highlight-{color}" data-highlight-colour="{color}">')
            replacement.append(changes)
            continue

        print("Trying to match line: ", line)
        # Match Release
        if re.compile(rf'^[\s]+<td>.+{RELEASE_PATTERN}</a></p></td>\n$').match(line):
            replacement.pop()
            return add_color_to_cells(lines, replacement, number_of_colums, 'green', i - 1, i + number_of_colums - 1)
        # Match Beta
        if re.compile(rf'^[\s]+<td>.+{BETA_PATTERN}</a></p></td>\n$').match(line):
            replacement.pop()
            return add_color_to_cells(lines, replacement, number_of_colums, 'yellow', i - 1, i + number_of_colums - 1)
        # Match Development
        if re.compile(rf'^[\s]+<td>.+{DEV_PATTERN}</a></p></td>\n$').match(line):
            replacement.pop()
            return add_color_to_cells(lines, replacement, number_of_colums, 'red', i - 1, i + number_of_colums - 1)

        replacement.append(line)

    new_str = ""
    for line in replacement:
        new_str = new_str + line
    return new_str

def pretty_names(bi):
    bi.product = product_friendly_names[bi.product] + " (" + bi.target + ")"
    bi.target = target_friendly_names[bi.target]

def update_product_page(cf, root_id, bi, template):
    create_child_page(cf, bi.target, root_id)
    current_id = cf.get_page_id(SPACE, bi.target)
    current_page = bi.target
    if bi.product:
        create_child_page(cf, bi.product, current_id)
        current_id = cf.get_page_id(SPACE, bi.product)
        current_page = bi.product

    page = cf.get_page_by_id(current_id, expand="body.storage")
    page_body = parse_page_body(page)

    html_url = f'<p><a href="{bi.url}">{bi.version}</a></p>'
    df = table_html_to_dataframe(page_body, ['Date', 'Version', 'Known Issues', 'Changes'])
    df = fix_values_from_conversion(df, parse_cells_from_table(page_body))
    df = append_data_to_dataframe(df, [bi.date, html_url, '', bi.changes])
    df = df.sort_values('Date', ascending=False)

    df_html = df.to_html(index=False, escape=False).replace("NaN", "")
    buf = io.StringIO(df_html)
    df_html = add_color_to_cells(buf.readlines(), [], len(df.columns))
    print("Before template : ", df_html)
    df_html = template.render({'Table': df_html, 'root': False})
    print("After template : ", df_html)

    cf.update_page(current_id, current_page, df_html)

def update_root_page(cf, root_id, bi, template):
    page = cf.get_page_by_id(root_id, expand="body.storage")
    page_body = parse_page_body(page)

    df = table_html_to_dataframe(page_body, ['Model', 'Variant', 'Version'])
    df = fix_hyperlink_from_conversion(df, 'Version', parse_href_from_table(page_body))
    if bi.product and bi.product not in df.values:
        html_url = f'<a href="{bi.url}">{bi.version}</a>'
        df = append_data_to_dataframe(df, [bi.target, bi.product, html_url])
    if bi.target and bi.target not in df.values:
        html_url = f'<a href="{bi.url}">{bi.version}</a>'
        df = append_data_to_dataframe(df, [bi.target, bi.product, html_url])
    df = update_data_in_dataframe(bi, df)

    df_html = df.to_html(index=False, escape=False).replace("NaN", "")
    df_html = template.render({'Table': df_html, 'root': True})

    cf.update_page(root_id, ROOT_PAGE, df_html)

def run(username, password, bi):
    cf = open_connection_to_confluence(username, password)

    loader = FileSystemLoader('pipelines/scripts/templates')
    env = Environment(loader=loader)
    template = env.get_template('confluence.tpl')

    pretty_names(bi)

    root_id = cf.get_page_id(SPACE, ROOT_PAGE)
    update_product_page(cf, root_id, bi, template)

    if re.compile(r'^' + RELEASE_PATTERN + r'$').match(bi.version):
        update_root_page(cf, root_id, bi, template)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("username", help="Username for Confluence")
    parser.add_argument("password", help="Password is the Confluence users API token ")
    parser.add_argument("target", help="This is the mcu target")
    parser.add_argument("version", help="This is the version of the build")
    parser.add_argument("url", help="This is the url used to download the build")
    parser.add_argument("-p", "--product", help="This is the product the target is built for")
    parser.add_argument("-c", "--changes", help="This is the url used to download the build")

    args = parser.parse_args()
    bi = BuildInfo(args.target, args.version, args.url, args.product, args.changes)

    print(args.username, args.password)
    run(args.username, args.password, bi)

if __name__ == "__main__":
    main()
