using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Forms;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace KegCommunicator {
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window {

        public List<string> ports;

        public MainWindow() {
            InitializeComponent();

            ports = new List<string>();
        }

        private void BC_send_Click(object sender, RoutedEventArgs e) {
            try {
                int red = Convert.ToInt32(BC_red.Text);
                int green = Convert.ToInt32(BC_green.Text);
                int blue = Convert.ToInt32(BC_blue.Text);

                SerialCommunicator sc = new SerialCommunicator(Port.Text, Convert.ToInt32(((ComboBoxItem)Baud.SelectedItem).Content.ToString()));
                BC_feedback.Text = sc.send_command("BC " + red + " " + green + " " + blue);
            }
            catch (InvalidCastException ex) {
                BC_feedback.Text = ex.Message;
            }
            catch (Exception ex) {
                BC_feedback.Text = ex.Message;
            }
        }

        private void GP_send_Click(object sender, RoutedEventArgs e) {
            try {
                SerialCommunicator sc = new SerialCommunicator(Port.Text, Convert.ToInt32(((ComboBoxItem)Baud.SelectedItem).Content.ToString()));
                if (Convert.ToBoolean(GP_left.IsChecked)) {
                    GP_feedback.Text = sc.send_command("GP L");
                }
                else {
                    GP_feedback.Text = sc.send_command("GP R");
                }
            }
            catch (Exception ex) {
                GP_feedback.Text = ex.Message;
            }
        }

        private void SB_send_Click(object sender, RoutedEventArgs e) {
            try {
                SerialCommunicator sc = new SerialCommunicator(Port.Text, Convert.ToInt32(((ComboBoxItem)Baud.SelectedItem).Content.ToString()));
                if (Convert.ToBoolean(SB_left.IsChecked)) {
                    SB_feedback.Text = sc.send_command("SB L " + SB_newBeer.Text);
                }
                else {
                    SB_feedback.Text = sc.send_command("SB R " + SB_newBeer.Text);
                }
            }
            catch (Exception ex) {
                SB_feedback.Text = ex.Message;
            }
        }

        private void GB_send_Click(object sender, RoutedEventArgs e) {
            try {
                SerialCommunicator sc = new SerialCommunicator(Port.Text, Convert.ToInt32(((ComboBoxItem)Baud.SelectedItem).Content.ToString()));
                if (Convert.ToBoolean(GB_left.IsChecked)) {
                    GB_feedback.Text = sc.send_command("GB L");
                }
                else {
                    GB_feedback.Text = sc.send_command("GB R");
                }
            }
            catch (Exception ex) {
                GB_feedback.Text = ex.Message;
            }
        }

        private void BC_colorPicker_Click(object sender, RoutedEventArgs e) {
            System.Windows.Forms.ColorDialog colorDialog =
                       new System.Windows.Forms.ColorDialog();
            colorDialog.AllowFullOpen = true;
            colorDialog.ShowDialog();

            BC_red.Text = Convert.ToString(colorDialog.Color.R);
            BC_green.Text = Convert.ToString(colorDialog.Color.G);
            BC_blue.Text = Convert.ToString(colorDialog.Color.B);
        }

        private void TabControl_SelectionChanged(object sender, SelectionChangedEventArgs e) {
            if (SV.IsSelected) {
                Title.Text = "Save State";
                e.Handled = true;
            }
            else if (BC.IsSelected) {
                Title.Text = "Set Backlight Color";
                e.Handled = true;
            }
            else if (RK.IsSelected) {
                Title.Text = "Reset Keg";
                e.Handled = true;
            }
            else if (SL.IsSelected) {
                Title.Text = "Set Keg Level (Beers)";
                e.Handled = true;
            }
            else if (SP.IsSelected) {
                Title.Text = "Set Keg Level (Pulses)";
                e.Handled = true;
            }
            else if (GP.IsSelected) {
                Title.Text = "Get Keg Level (Pulses)";
                e.Handled = true;
            }
            else if (SB.IsSelected) {
                Title.Text = "Set Beer Name";
                e.Handled = true;
            }
            else if (GB.IsSelected) {
                Title.Text = "Get Beer Name";
                e.Handled = true;
            }
        }

        private void PortRefresh_Click(object sender, RoutedEventArgs e) {
            ports = new List<string>(SerialPort.GetPortNames());
            Port.ItemsSource = ports;

            try {
                Port.SelectedIndex = 0;
            }
            catch (Exception ex) { }
        }
    }
}
