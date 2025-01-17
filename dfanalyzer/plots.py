import dask
import matplotlib.pyplot as plt
import numpy as np
import dask.dataframe as dd
import pandas as pd
from matplotlib import ticker
from typing import Literal, Tuple
import seaborn as sns

TIME_COLS = ['io_time', 'app_io_time']


class DFAnalyzerPlots(object):

    def __init__(self, events: dd.DataFrame, slope_threshold: int, time_granularity: int) -> None:
        self.events = events
        self.slope_threshold = slope_threshold
        self.time_granularity = time_granularity
        
    def create_event_timeline(self, df):
        # Filter for start and end events
        start_events = df[df['name'] == 'start']
        end_events = df[df['name'] == 'end']

        # Merging start and end events on pid
        merged_events = start_events[['pid', 'ts']].rename(columns={'ts': 'start_time'}).merge(
            end_events[['pid', 'ts']].rename(columns={'ts': 'end_time'}),
            on='pid',
            how='inner'
        )

        # Convert microseconds to seconds
        merged_events['start_time'] = merged_events['start_time'] / 1_000_000  # Convert to seconds
        merged_events['end_time'] = merged_events['end_time'] / 1_000_000      # Convert to seconds

        # Convert PIDs to strings for better labeling
        merged_events['pid'] = merged_events['pid'].astype(str)

        # Create the plot
        plt.figure(figsize=(10, 6))
        
        for idx, row in merged_events.iterrows():
            plt.hlines(y=row['pid'], xmin=row['start_time'], xmax=row['end_time'], color='blue', linewidth=2)

        plt.xlabel('Time (seconds)')
        plt.ylabel('PID')
        plt.title('Event Timeline')
        plt.grid(True)

        plt.show()

    def time_bw_timeline(
        self,
        time_col: Literal['io_time', 'app_io_time'],
        figsize: Tuple[int, int],
        bw_unit: Literal['kb', 'mb', 'gb','tb'] = 'kb',
        line1_label: str = 'I/O Time',
        line2_label: str = 'I/O Bandwidth',
        xlabel: str = 'Timeline (sec)',
        y1label: str = 'Time (sec)',
        y2label: str = 'Bandwidth',
        x_num_ticks: int = 10,
        y_num_ticks: int = 5,
        y_axis_formatter = ticker.FuncFormatter(lambda x, pos: '{:.0f}'.format(x)),
    ):
        size_denom = 1024
        y2label_suffix = 'KB/s'
        if bw_unit == 'mb':
            size_denom = 1024 ** 2
            y2label_suffix = 'MB/s'
        elif bw_unit == 'gb':
            size_denom = 1024 ** 3
            y2label_suffix = 'GB/s'
        elif bw_unit == 'tb':
            size_denom = 1024 ** 4
            y2label_suffix = 'TB/s'

        def _set_bw(df: pd.DataFrame):
            for col in TIME_COLS:
                df[f"{col}_bw"] = (df['size'] / size_denom) / (df[col] / 1e6)
            df.replace([np.inf, -np.inf], np.nan, inplace=True)
            df[f"{col}_bw"] = df[f"{col}_bw"].fillna(0)
            return df

        timeline = self._create_timeline(events=self.events) \
                    .reset_index() \
                    .map_partitions(_set_bw) \
                    .compute() \
                    .assign(seconds=self._assign_seconds) \
                    .sort_values("seconds")

        fig, ax1 = plt.subplots(figsize=figsize)
        if time_col == "io_time":
            phase = 2
        else:
            phase = 3
        timeline.query(f"phase == {phase}").plot.line(ax=ax1, x='seconds', y=time_col,
                           alpha=0.8, color='C0', label=line1_label)

        ax2 = ax1.twinx()
        y2_min = np.min([timeline.query(f"phase == {phase}")[f"{time_col}_bw"].min()])
        y2_max = np.max([timeline.query(f"phase == {phase}")[f"{time_col}_bw"].max()])

        has_y2 = False
        if y2_max > 0:
            timeline.query(f"phase == {phase}").plot.line(ax=ax2, x='seconds', y=f"{time_col}_bw", linestyle='dashed',
                               alpha=0.8, color='C1', label=line2_label)
            y2_min = np.min([timeline[f"{time_col}_bw"].min()])
            y2_max = np.max([timeline[f"{time_col}_bw"].max()])
            ax2.set_ylim([0, y2_max])
            has_y2 = True
        y1_min = np.min([timeline[col].min() for col in TIME_COLS])
        y1_max = np.max([timeline[col].max() for col in TIME_COLS])
        ax1.set_ylim([0, y1_max])



        ax1.yaxis.set_major_formatter(ticker.FuncFormatter(
            lambda x, pos: '{:.1f}'.format(x/1e6)))
        ax1.set_xlabel(xlabel)
        ax1.set_ylabel(y1label)

        ax2.yaxis.set_major_formatter(y_axis_formatter)
        if has_y2:
            ax2.set_ylabel(f"{y2label} ({y2label_suffix})")

        ax1.yaxis.set_major_locator(ticker.LinearLocator(y_num_ticks))
        if has_y2:
            ax2.yaxis.set_major_locator(ticker.LinearLocator(y_num_ticks))

        handles1, labels1 = ax1.get_legend_handles_labels()
        handles2, labels2 = ax2.get_legend_handles_labels()
        ax1.get_legend().remove()
        if has_y2:
            ax2.get_legend().remove()

        # Combine handles and labels
        handles = handles1 + handles2
        labels = labels1 + labels2

        if has_y2:
            # Create the legend
            ax2.legend(handles, labels)

        ax1.minorticks_on()
        if has_y2:
            ax2.minorticks_on()

        ax1.grid(axis='y', which='major')
        ax1.grid(axis='y', which='minor', alpha=0.3)

        ts_min, te_max = dask.compute(
            self.events['ts'].min(), self.events['te'].max())
        ax1.set_xlim([ts_min / 1e6, te_max / 1e6])
        ax1.xaxis.set_major_formatter(ticker.FormatStrFormatter('%d'))
        ax1.xaxis.set_major_locator(ticker.LinearLocator(x_num_ticks))

        return fig, ax1, ax2

    def xfer_size_timeline(
        self,
        figsize: Tuple[int, int],
        unit: Literal['kb', 'mb', 'gb'] = 'kb',
        xlabel: str = 'Timeline (sec)',
        ylabel: str = 'Transfer Size',
        x_num_ticks: int = 10,
        y_num_ticks: int = 5,
        y_axis_formatter = ticker.FuncFormatter(lambda x, pos: '{:.0f}'.format(x)),
    ):
        xfer_col = 'xfer'

        ylabel_denom = 1024
        ylabel_unit = 'KB'
        if unit == 'mb':
            ylabel_denom = 1024 ** 2
            ylabel_unit = 'MB'
        elif unit == 'gb':
            ylabel_denom = 1024 ** 3
            ylabel_unit = 'GB'

        def _set_xfer_size(df: pd.DataFrame):
            df[xfer_col] = df['size'] / ylabel_denom /df['index']
            return df

        timeline = self._create_timeline(events=self.events) \
            .reset_index() \
            .query("phase == 2") \
            .map_partitions(_set_xfer_size) \
            .compute() \
            .assign(seconds=self._assign_seconds) \
            .sort_values("seconds")

        fig, ax = plt.subplots(figsize=figsize)

        timeline.plot.line(ax=ax, x='seconds', y=xfer_col,
                           alpha=0.8, color='C3', label='Avg. Transfer Size')

        ax.set_xlabel(xlabel)
        ax.set_ylabel(f"{ylabel} ({ylabel_unit})")
        _, ylim_max = ax.get_ylim()
        ax.set_ylim([0, ylim_max * 2])
        ax.yaxis.set_major_locator(ticker.LinearLocator(y_num_ticks))
        # ax1.yaxis.set_major_formatter(ticker.FuncFormatter(
        #     lambda x, pos: '{:.0f}'.format(x/1e6)))
        ax.yaxis.set_major_formatter(y_axis_formatter)

        # ax.get_legend().remove()
        ax.minorticks_on()
        ax.grid(axis='y', which='major')
        ax.grid(axis='y', which='minor', alpha=0.3)

        ts_min, te_max = dask.compute(
            self.events['ts'].min(), self.events['te'].max())
        ax.set_xlim([ts_min / 1e6, te_max / 1e6])
        ax.xaxis.set_major_formatter(ticker.FormatStrFormatter('%d'))
        ax.xaxis.set_major_locator(ticker.LinearLocator(x_num_ticks))

        return fig, ax

    def _assign_seconds(self, timeline: pd.DataFrame):
        return timeline['trange'] * (self.time_granularity / 1e6)

    @staticmethod
    def _color_map(threshold: float):
        if threshold >= 0.9:
            return 'red'
        elif threshold >= 0.75:
            return 'darkorange'
        elif threshold >= 0.5:
            return 'orange'
        elif threshold >= 0.25:
            return 'gold'
        elif threshold >= 0.1:
            return 'yellow'
        elif threshold >= 0.01:
            return 'yellowgreen'
        elif threshold >= 0.001:
            return 'limegreen'
        else:
            return 'green'

    @staticmethod
    def _create_timeline(events: dd.DataFrame):
        events['index'] = events['size']
        timeline = events.groupby(['phase','trange', 'pid', 'tid']) \
            .agg({
                'index': 'count',
                'size': 'sum',
                'io_time': 'sum',
                'app_io_time': 'sum',
            }) \
            .groupby(['phase','trange']) \
            .agg({
                'index': 'sum',
                'size': 'sum',
                'io_time': max,
                'app_io_time': max,
            }).reset_index().set_index("trange", sorted=True)

        return timeline


# plots used for graph based methods 
class GrepIOPlots:
    def histogram(self,data,x_label):
        # Create the plot with logarithmic y-axis and linear x-axis
        plt.hist(data, bins=23, color='skyblue', edgecolor='black')
        plt.yscale('log')  # Set logarithmic scale for the y-axis
        # Add labels and title
        plt.xlabel(x_label)
        plt.ylabel('Frequency (log scale)')  # Adjust the label for the y-axis
        plt.title('Histogram of '+x_label)
        # Show the plot
        plt.show()

    def bar_graph(self,data,xlabel):
        '''
        Example:
        value_counts = analyzer_mummi1.events['Name'].value_counts().compute()
        bar_graph(value_counts,"Name")
        '''
        # Extract values and counts
        values = data.index.tolist()
        counts = data.values.tolist()
        # Create a bar graph
        plt.figure(figsize=(10, 6))
        plt.bar(values, counts)
        # Add numerical values on the bars
        for i, (xi, yi) in enumerate(zip(values, counts)):
            plt.text(xi, yi, f'{yi:.2f}', ha='center', va='bottom',fontsize=6)
        # Add labels and title
        plt.xlabel('Values')
        plt.ylabel('Counts')
        plt.title("Bar Graph of "+xlabel+" counts")
        # Rotate x-axis labels if needed
        plt.xticks(rotation=45)
        # Show the plot
        plt.show()

    def line_plot(self, ddf):
        '''
        Function: line_graph (Divides the sorted dataframe into 10)
        Usage:
        line_plot(sorted_events)
        '''
        idx = np.linspace(0, len(ddf)-1,10, dtype=int)
        ts = [ddf.compute().iloc[k]['ts'] for k in idx]

        plt.plot(ts,idx, linestyle='--',marker= 'o')
        # Set logarithmic scales for both axes
        # plt.xscale('log')
        # plt.yscale('log')
        # Add labels and title
        plt.xlabel('Time (ts)')
        plt.ylabel('Index in Dataframe')
        plt.title('Index vs Time')
        plt.show()

    def hmap(self, mp=None, df=None, size_bins=[], size_labels=[], degree_bins=[], degree_labels = []):
        # df= inter.compute()
        if mp:
            df= df[df['mount_point']==mp]
        else:
            mp = "All Mount Points"
        # size_bins, size_labels, degree_bins, degree_labels = binned_category()
        df['size_category'] = pd.cut(df['size'], bins=size_bins, labels=size_labels, right=False)
        df['deg_category'] = pd.cut(df['deg_caller'], bins=degree_bins, labels=degree_labels, right=False)

        df['interference'] = df['interference'].astype(float)

        all_combinations = pd.MultiIndex.from_product([df['size_category'].unique(), df['deg_category'].unique()], names=['size_category', 'deg_category'])
        merged = pd.merge(df, pd.DataFrame(index=all_combinations).reset_index(), on=['size_category', 'deg_category'], how='left')
        grouped = merged.groupby(['size_category', 'deg_category']).agg({'interference': 'mean'}).reset_index()

        pivot_table = grouped.pivot(index='size_category', columns='deg_category', values='interference').fillna(-0.5)
        # Create heatmap using seaborn
        plt.figure(figsize=(8, 6))
        pivot_table = 1/pivot_table
        sns.heatmap(pivot_table, annot=True, cmap='YlGnBu', fmt='.1f', linewidths=.5)
        plt.title(f'Average Interference Heatmap {mp}')
        plt.xlabel('Degrees')
        plt.ylabel('Sizes')
        plt.show()
    
    def correlation(self, ddf, group_col = '', col_a = '', col_b = '' ):
        grouped = ddf.groupby(group_col).apply(
        lambda x: x[col_a].corr(x[col_b]),
            meta=('correlation', 'f8')
        ).compute()
        # grouped = ddf.groupby('trange').apply(
        # lambda x: x['dur'].corr(x['interference']),
        #     meta=('correlation', 'f8')
        # ).compute()
        # 1. Binning the trange values
        bin_size = 50  # Adjust this value as needed
        grouped_binned = grouped.groupby(grouped.index // bin_size).mean()
        plt.figure(figsize=(15, 6))
        plt.plot(grouped_binned.index * bin_size, grouped_binned.values, marker='o')
        plt.title('Average Correlation ({col_a} and {col_b})')
        plt.xlabel(f'{group_col} (binned)')
        plt.ylabel('Correlation')
        plt.grid(True, linestyle='--', alpha=0.7)
        plt.tight_layout()
        plt.show()

    def correlation_plot(self, grouped, bin_size = 50, group_col = '', col_a = '', col_b = '' ):
        # grouped = ddf.groupby(group_col).apply(
        # lambda x: x[col_a].corr(x[col_b]),
        #     meta=('correlation', 'f8')
        # ).compute()
        # grouped = ddf.groupby('trange').apply(
        # lambda x: x['dur'].corr(x['interference']),
        #     meta=('correlation', 'f8')
        # ).compute()
        # 1. Binning the trange values
        # bin_size = 50  # Adjust this value as needed
        grouped_binned = grouped.groupby(grouped.index // bin_size).mean()
        plt.figure(figsize=(15, 6))
        plt.plot(grouped_binned.index * bin_size, grouped_binned.values, marker='o')
        plt.title(f'Average Correlation ({col_a} and {col_b})')
        plt.xlabel(f'{group_col} (binned)')
        plt.ylabel('Correlation')
        plt.grid(True, linestyle='--', alpha=0.7)
        plt.tight_layout()
        plt.show()
        