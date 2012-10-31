using System;
using System.Collections.Generic;
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
        public MainWindow() {
            InitializeComponent();
        }

        private void BC_send_Click(object sender, RoutedEventArgs e) {
            try {
                int red = Convert.ToInt32(BC_red.Text);
                int green = Convert.ToInt32(BC_green.Text);
                int blue = Convert.ToInt32(BC_blue.Text);

                SerialCommunicator sc = new SerialCommunicator(Port.Text, Convert.ToInt32(Baud.Text));
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
                SerialCommunicator sc = new SerialCommunicator(Port.Text, Convert.ToInt32(Baud.Text));
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
                SerialCommunicator sc = new SerialCommunicator(Port.Text, Convert.ToInt32(Baud.Text));
                if (Convert.ToBoolean(SB_left.IsChecked)) {
                    SB_feedback.Text = sc.send_command("SB L " + SB_newBeer);
                }
                else {
                    SB_feedback.Text = sc.send_command("SB R " + SB_newBeer);
                }
            }
            catch (Exception ex) {
                SB_feedback.Text = ex.Message;
            }
        }

        private void Button_Click_1(object sender, RoutedEventArgs e) {
            try {
                SerialCommunicator sc = new SerialCommunicator(Port.Text, Convert.ToInt32(Baud.Text));
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
    }
}
