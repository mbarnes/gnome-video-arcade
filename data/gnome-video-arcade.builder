<?xml version="1.0" encoding="UTF-8"?>
<interface>
  <!-- interface-requires gtk+ 3.0 -->
  <!-- interface-requires gva 0.0 -->
  <object class="GtkActionGroup" id="lockable-actions">
    <child>
      <object class="GtkAction" id="insert-favorite">
        <property name="label" translatable="yes">Add to _Favorites</property>
        <property name="stock_id">gtk-add</property>
        <property name="icon_name">Add the selected game to my list of favorites</property>
        <signal name="activate" handler="gva_action_insert_favorite_cb" swapped="no"/>
      </object>
      <accelerator key="plus" modifiers="GDK_CONTROL_MASK"/>
    </child>
    <child>
      <object class="GtkAction" id="next-game">
        <property name="tooltip" translatable="yes">Show next game</property>
        <property name="stock_id">gtk-go-forward</property>
        <signal name="activate" handler="gva_action_next_game_cb" swapped="no"/>
      </object>
      <accelerator key="Right" modifiers="GDK_MOD1_MASK"/>
    </child>
    <child>
      <object class="GtkAction" id="play-back">
        <property name="label" translatable="yes">Play _Back...</property>
        <property name="tooltip" translatable="yes">Play back the selected game recording</property>
        <property name="stock_id">gtk-media-play</property>
        <signal name="activate" handler="gva_action_play_back_cb" swapped="no"/>
      </object>
    </child>
    <child>
      <object class="GtkAction" id="previous-game">
        <property name="tooltip" translatable="yes">Show previous game</property>
        <property name="stock_id">gtk-go-back</property>
        <signal name="activate" handler="gva_action_previous_game_cb" swapped="no"/>
      </object>
      <accelerator key="Left" modifiers="GDK_MOD1_MASK"/>
    </child>
    <child>
      <object class="GtkAction" id="properties">
        <property name="label" translatable="yes">_Properties</property>
        <property name="tooltip" translatable="yes">Show information about the selected game</property>
        <property name="stock_id">gtk-properties</property>
        <signal name="activate" handler="gva_action_properties_cb" swapped="no"/>
      </object>
      <accelerator key="p" modifiers="GDK_CONTROL_MASK"/>
    </child>
    <child>
      <object class="GtkAction" id="record">
        <property name="label" translatable="yes">_Record</property>
        <property name="tooltip" translatable="yes">Start the selected game and record keypresses to a file</property>
        <property name="stock_id">gtk-media-record</property>
        <signal name="activate" handler="gva_action_record_cb" swapped="no"/>
      </object>
      <accelerator key="r" modifiers="GDK_CONTROL_MASK"/>
    </child>
    <child>
      <object class="GtkAction" id="remove-column">
        <property name="label">(Label is set at runtime)</property>
        <property name="tooltip">(Tooltip is set at runtime)</property>
        <signal name="activate" handler="gva_action_remove_column_cb" swapped="no"/>
      </object>
    </child>
    <child>
      <object class="GtkAction" id="remove-favorite">
        <property name="label" translatable="yes">Remove from _Favorites</property>
        <property name="tooltip" translatable="yes">Remove the selected game from my list of favorites</property>
        <property name="stock_id">gtk-remove</property>
        <signal name="activate" handler="gva_action_remove_favorite_cb" swapped="no"/>
      </object>
      <accelerator key="minus" modifiers="GDK_CONTROL_MASK"/>
    </child>
    <child>
      <object class="GtkAction" id="save-errors">
        <property name="label" translatable="yes">Save _As...</property>
        <property name="tooltip" translatable="yes">Save ROM errors to a file</property>
        <property name="stock_id">gtk-save-as</property>
        <signal name="activate" handler="gva_action_save_errors_cb" swapped="no"/>
      </object>
    </child>
    <child>
      <object class="GtkAction" id="search">
        <property name="label" translatable="yes">S_earch...</property>
        <property name="tooltip" translatable="yes">Show a custom list of games</property>
        <property name="stock_id">gtk-find</property>
        <signal name="activate" handler="gva_action_search_cb" swapped="no"/>
      </object>
    </child>
    <child>
      <object class="GtkAction" id="show-play-back">
        <property name="label" translatable="yes">Play _Back...</property>
        <property name="tooltip" translatable="yes">Play back a previously recorded game</property>
        <property name="stock_id">gtk-media-play</property>
        <signal name="activate" handler="gva_action_show_play_back_cb" swapped="no"/>
      </object>
    </child>
    <child>
      <object class="GtkAction" id="start">
        <property name="label" translatable="yes">_Start</property>
        <property name="tooltip" translatable="yes">Start the selected game</property>
        <property name="stock_id">gtk-execute</property>
        <signal name="activate" handler="gva_action_start_cb" swapped="no"/>
      </object>
      <accelerator key="s" modifiers="GDK_CONTROL_MASK"/>
    </child>
    <child>
      <object class="GtkToggleAction" id="show-clones">
        <property name="label" translatable="yes">Show _alternate versions of original games</property>
        <signal name="toggled" handler="gva_action_show_clones_cb" swapped="no"/>
      </object>
    </child>
    <child>
      <object class="GtkRadioAction" id="view-inital">
        <property name="label">(This is a bogus view that we switch away from on startup.)</property>
        <property name="draw_as_radio">True</property>
        <property name="active">True</property>
        <property name="value">-1</property>
        <signal name="changed" handler="gva_action_view_changed_cb" swapped="no"/>
      </object>
    </child>
    <child>
      <object class="GtkRadioAction" id="view-available">
        <property name="label" translatable="yes">_Available Games</property>
        <property name="tooltip" translatable="yes">Show all available games</property>
        <property name="draw_as_radio">True</property>
        <property name="group">view-inital</property>
      </object>
      <accelerator key="1" modifiers="GDK_CONTROL_MASK"/>
    </child>
    <child>
      <object class="GtkRadioAction" id="view-favorites">
        <property name="label" translatable="yes">_Favorite Games</property>
        <property name="tooltip" translatable="yes">Only show my favorite games</property>
        <property name="draw_as_radio">True</property>
        <property name="value">1</property>
        <property name="group">view-inital</property>
      </object>
      <accelerator key="2" modifiers="GDK_CONTROL_MASK"/>
    </child>
    <child>
      <object class="GtkRadioAction" id="view-results">
        <property name="label" translatable="yes">Search _Results</property>
        <property name="tooltip" translatable="yes">Show my search results</property>
        <property name="draw_as_radio">True</property>
        <property name="value">2</property>
        <property name="group">view-inital</property>
      </object>
      <accelerator key="3" modifiers="GDK_CONTROL_MASK"/>
    </child>
  </object>
  <object class="GtkActionGroup" id="standard-actions">
    <child>
      <object class="GtkAction" id="about">
        <property name="label" translatable="yes">_About</property>
        <property name="tooltip" translatable="yes">Show information about the application</property>
        <property name="stock_id">gtk-about</property>
        <signal name="activate" handler="gva_action_about_cb" swapped="no"/>
      </object>
    </child>
    <child>
      <object class="GtkAction" id="contents">
        <property name="label" translatable="yes">_Contents</property>
        <property name="tooltip" translatable="yes">Open the help documentation</property>
        <property name="stock_id">gtk-help</property>
        <signal name="activate" handler="gva_action_contents_cb" swapped="no"/>
      </object>
    </child>
    <child>
      <object class="GtkAction" id="preferences">
        <property name="label" translatable="yes">_Preferences</property>
        <property name="tooltip" translatable="yes">Configure the application</property>
        <property name="stock_id">gtk-preferences</property>
        <signal name="activate" handler="gva_action_preferences_cb" swapped="no"/>
      </object>
    </child>
    <child>
      <object class="GtkAction" id="quit">
        <property name="label" translatable="yes">_Quit</property>
        <property name="tooltip" translatable="yes">Quit the application</property>
        <property name="stock_id">gtk-quit</property>
        <signal name="activate" handler="gva_action_quit_cb" swapped="no"/>
      </object>
    </child>
    <child>
      <object class="GtkAction" id="add-column-menu">
        <property name="label" translatable="yes">_Add Column</property>
      </object>
    </child>
    <child>
      <object class="GtkAction" id="edit-menu">
        <property name="label" translatable="yes">_Edit</property>
      </object>
    </child>
    <child>
      <object class="GtkAction" id="game-menu">
        <property name="label" translatable="yes">_Game</property>
      </object>
    </child>
    <child>
      <object class="GtkAction" id="help-menu">
        <property name="label" translatable="yes">_Help</property>
      </object>
    </child>
    <child>
      <object class="GtkAction" id="view-menu">
        <property name="label" translatable="yes">_View</property>
      </object>
    </child>
    <child>
      <object class="GtkToggleAction" id="auto-play">
        <property name="label" translatable="yes">Play music _automatically</property>
        <property name="tooltip" translatable="yes">Automatically play a music clip from the selected game</property>
      </object>
    </child>
    <child>
      <object class="GtkToggleAction" id="auto-save">
        <property name="label" translatable="yes">_Restore previous state when starting a game</property>
      </object>
    </child>
    <child>
      <object class="GtkToggleAction" id="full-screen">
        <property name="label" translatable="yes">Start games in _fullscreen mode</property>
      </object>
    </child>
  </object>
  <object class="GtkWindow" id="audit-window">
    <property name="width_request">400</property>
    <property name="can_focus">False</property>
    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
    <property name="title" translatable="yes"> </property>
    <property name="resizable">False</property>
    <property name="window_position">center-on-parent</property>
    <property name="destroy_with_parent">True</property>
    <property name="type_hint">dialog</property>
    <property name="urgency_hint">True</property>
    <property name="transient_for">main-window</property>
    <child>
      <object class="GtkTable" id="audit-table">
        <property name="visible">True</property>
        <property name="can_focus">False</property>
        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
        <property name="border_width">12</property>
        <property name="n_rows">2</property>
        <property name="n_columns">2</property>
        <property name="column_spacing">12</property>
        <property name="row_spacing">12</property>
        <child>
          <object class="GtkImage" id="audit-image">
            <property name="visible">True</property>
            <property name="can_focus">False</property>
            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
            <property name="yalign">0</property>
            <property name="stock">gtk-dialog-warning</property>
            <property name="icon-size">6</property>
          </object>
          <packing>
            <property name="x_options"></property>
            <property name="y_options">GTK_FILL</property>
          </packing>
        </child>
        <child>
          <object class="GtkBox" id="audit-vbox">
            <property name="visible">True</property>
            <property name="can_focus">False</property>
            <property name="orientation">vertical</property>
            <property name="spacing">12</property>
            <child>
              <object class="GtkLabel" id="audit-title">
                <property name="visible">True</property>
                <property name="can_focus">False</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="xalign">0</property>
                <property name="yalign">0</property>
                <property name="label" translatable="yes">&lt;big&gt;&lt;b&gt;Errors detected in ROM files&lt;/b&gt;&lt;/big&gt;</property>
                <property name="use_markup">True</property>
                <property name="wrap">True</property>
              </object>
              <packing>
                <property name="expand">False</property>
                <property name="fill">True</property>
                <property name="position">0</property>
              </packing>
            </child>
            <child>
              <object class="GtkLabel" id="audit-verbage">
                <property name="visible">True</property>
                <property name="can_focus">False</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="xalign">0</property>
                <property name="yalign">0</property>
                <property name="label" translatable="yes">An audit of the available games has detected errors in some of the ROM files. The following games will not be available in GNOME Video Arcade.</property>
                <property name="use_markup">True</property>
                <property name="wrap">True</property>
              </object>
              <packing>
                <property name="expand">False</property>
                <property name="fill">True</property>
                <property name="position">1</property>
              </packing>
            </child>
            <child>
              <object class="GtkHBox" id="audit-info-hbox">
                <property name="visible">True</property>
                <property name="can_focus">False</property>
                <property name="spacing">6</property>
                <child>
                  <object class="GtkImage" id="audit-info-image">
                    <property name="visible">True</property>
                    <property name="can_focus">False</property>
                    <property name="yalign">0</property>
                    <property name="stock">gtk-dialog-info</property>
                  </object>
                  <packing>
                    <property name="expand">False</property>
                    <property name="fill">False</property>
                    <property name="position">0</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkLabel" id="audit-info-label">
                    <property name="visible">True</property>
                    <property name="can_focus">False</property>
                    <property name="xalign">0</property>
                    <property name="yalign">0</property>
                    <property name="label" translatable="yes">&lt;small&gt;&lt;i&gt;If these games worked with previous versions of MAME, it is likely their ROM requirements have changed.  If so, you will need to acquire more recent ROM files.&lt;/i&gt;&lt;/small&gt;</property>
                    <property name="use_markup">True</property>
                    <property name="wrap">True</property>
                  </object>
                  <packing>
                    <property name="expand">False</property>
                    <property name="fill">False</property>
                    <property name="position">1</property>
                  </packing>
                </child>
              </object>
              <packing>
                <property name="expand">False</property>
                <property name="fill">True</property>
                <property name="position">2</property>
              </packing>
            </child>
            <child>
              <object class="GtkScrolledWindow" id="audit-scrolled-window">
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="shadow_type">in</property>
                <child>
                  <object class="GtkTreeView" id="audit-tree-view">
                    <property name="visible">True</property>
                    <property name="can_focus">True</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="headers_visible">False</property>
                    <child internal-child="selection">
                      <object class="GtkTreeSelection" id="treeview-selection"/>
                    </child>
                  </object>
                </child>
              </object>
              <packing>
                <property name="expand">True</property>
                <property name="fill">True</property>
                <property name="position">3</property>
              </packing>
            </child>
          </object>
          <packing>
            <property name="left_attach">1</property>
            <property name="right_attach">2</property>
          </packing>
        </child>
        <child>
          <object class="GtkHButtonBox" id="audit-button-box">
            <property name="visible">True</property>
            <property name="can_focus">False</property>
            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
            <property name="spacing">12</property>
            <property name="homogeneous">True</property>
            <property name="layout_style">end</property>
            <child>
              <object class="GtkButton" id="audit-save-button">
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="receives_default">True</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="has_tooltip">True</property>
                <property name="tooltip_markup" translatable="yes">Save ROM errors to a file</property>
                <property name="tooltip_text" translatable="yes">Save ROM errors to a file</property>
                <property name="related_action">save-errors</property>
                <property name="use_stock">True</property>
              </object>
              <packing>
                <property name="expand">False</property>
                <property name="fill">False</property>
                <property name="position">0</property>
              </packing>
            </child>
            <child>
              <object class="GtkButton" id="audit-close-button">
                <property name="label">gtk-close</property>
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="can_default">True</property>
                <property name="has_default">True</property>
                <property name="receives_default">True</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="has_tooltip">True</property>
                <property name="tooltip_markup" translatable="yes">Close this window</property>
                <property name="tooltip_text" translatable="yes">Close this window</property>
                <property name="use_action_appearance">False</property>
                <property name="use_stock">True</property>
                <signal name="clicked" handler="gtk_widget_hide" object="audit-window" swapped="yes"/>
              </object>
              <packing>
                <property name="expand">False</property>
                <property name="fill">False</property>
                <property name="position">1</property>
              </packing>
            </child>
          </object>
          <packing>
            <property name="right_attach">2</property>
            <property name="top_attach">1</property>
            <property name="bottom_attach">2</property>
            <property name="y_options"></property>
          </packing>
        </child>
      </object>
    </child>
  </object>
  <object class="GtkImage" id="main-start-game-image">
    <property name="visible">True</property>
    <property name="can_focus">False</property>
    <property name="stock">gtk-execute</property>
  </object>
  <object class="GtkWindow" id="main-window">
    <property name="can_focus">False</property>
    <property name="title" translatable="yes">GNOME Video Arcade</property>
    <property name="default_width">500</property>
    <property name="default_height">400</property>
    <property name="icon_name">gnome-video-arcade</property>
    <signal name="destroy" handler="gva_main_window_destroy_cb" swapped="no"/>
    <signal name="window-state-event" handler="gva_main_window_window_state_event_cb" swapped="no"/>
    <signal name="configure-event" handler="gva_main_window_configure_event_cb" swapped="no"/>
    <child>
      <object class="GtkVBox" id="main-vbox">
        <property name="visible">True</property>
        <property name="can_focus">False</property>
        <child>
          <object class="GtkVBox" id="main-contents">
            <property name="visible">True</property>
            <property name="can_focus">False</property>
            <property name="border_width">12</property>
            <property name="spacing">12</property>
            <child>
              <object class="GtkHBox" id="main-top-hbox">
                <property name="visible">True</property>
                <property name="can_focus">False</property>
                <property name="spacing">12</property>
                <child>
                  <object class="GtkHButtonBox" id="main-top-button-box">
                    <property name="visible">True</property>
                    <property name="can_focus">False</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="spacing">3</property>
                    <property name="homogeneous">True</property>
                    <property name="layout_style">start</property>
                    <child>
                      <object class="GtkToggleButton" id="main-view-button-0">
                        <property name="visible">True</property>
                        <property name="can_focus">True</property>
                        <property name="receives_default">True</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="tooltip_text" translatable="yes">Show all available games</property>
                        <property name="related_action">view-available</property>
                      </object>
                      <packing>
                        <property name="expand">False</property>
                        <property name="fill">False</property>
                        <property name="position">0</property>
                      </packing>
                    </child>
                    <child>
                      <object class="GtkToggleButton" id="main-view-button-1">
                        <property name="visible">True</property>
                        <property name="can_focus">True</property>
                        <property name="receives_default">True</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="tooltip_text" translatable="yes">Only show my favorite games</property>
                        <property name="related_action">view-favorites</property>
                      </object>
                      <packing>
                        <property name="expand">False</property>
                        <property name="fill">False</property>
                        <property name="position">1</property>
                      </packing>
                    </child>
                    <child>
                      <object class="GtkToggleButton" id="main-view-button-2">
                        <property name="visible">True</property>
                        <property name="can_focus">True</property>
                        <property name="receives_default">True</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="tooltip_text" translatable="yes">Show my search results</property>
                        <property name="related_action">view-results</property>
                      </object>
                      <packing>
                        <property name="expand">False</property>
                        <property name="fill">False</property>
                        <property name="position">2</property>
                      </packing>
                    </child>
                  </object>
                  <packing>
                    <property name="expand">True</property>
                    <property name="fill">True</property>
                    <property name="position">0</property>
                  </packing>
                </child>
                <child>
                  <object class="GvaMuteButton" id="main-mute-button">
                    <property name="visible">True</property>
                    <property name="can_focus">False</property>
                    <property name="receives_default">True</property>
                    <property name="has_tooltip">True</property>
                    <property name="tooltip_markup">In-game sound is enabled</property>
                    <property name="tooltip_text">In-game sound is enabled</property>
                    <property name="use_action_appearance">False</property>
                    <property name="relief">none</property>
                  </object>
                  <packing>
                    <property name="expand">False</property>
                    <property name="fill">False</property>
                    <property name="position">1</property>
                  </packing>
                </child>
              </object>
              <packing>
                <property name="expand">False</property>
                <property name="fill">False</property>
                <property name="position">0</property>
              </packing>
            </child>
            <child>
              <object class="GtkVBox" id="main-center-vbox">
                <property name="visible">True</property>
                <property name="can_focus">False</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="spacing">2</property>
                <child>
                  <object class="GtkScrolledWindow" id="main-scrolled-window">
                    <property name="visible">True</property>
                    <property name="can_focus">True</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="shadow_type">in</property>
                    <child>
                      <object class="GtkTreeView" id="main-tree-view">
                        <property name="visible">True</property>
                        <property name="sensitive">False</property>
                        <property name="can_focus">True</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="has_tooltip">True</property>
                        <property name="rules_hint">True</property>
                        <property name="show_expanders">False</property>
                        <signal name="button-press-event" handler="gva_tree_view_button_press_event_cb" swapped="no"/>
                        <signal name="row-activated" handler="gva_tree_view_row_activated_cb" swapped="no"/>
                        <signal name="query-tooltip" handler="gva_tree_view_query_tooltip_cb" swapped="no"/>
                        <signal name="popup-menu" handler="gva_tree_view_popup_menu_cb" swapped="no"/>
                        <child internal-child="selection">
                          <object class="GtkTreeSelection" id="treeview-selection1"/>
                        </child>
                      </object>
                    </child>
                  </object>
                  <packing>
                    <property name="expand">True</property>
                    <property name="fill">True</property>
                    <property name="position">0</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkHBox" id="main-search-hbox">
                    <property name="can_focus">False</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="has_tooltip">True</property>
                    <property name="spacing">6</property>
                    <signal name="query-tooltip" handler="gva_main_search_query_tooltip_cb" swapped="no"/>
                    <child>
                      <object class="GtkLabel" id="main-search-label">
                        <property name="visible">True</property>
                        <property name="can_focus">False</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="xalign">0</property>
                        <property name="label" translatable="yes" comments="This label precedes the search entry. e.g. Search for: Pac-Man">Search for:</property>
                      </object>
                      <packing>
                        <property name="expand">False</property>
                        <property name="fill">False</property>
                        <property name="position">0</property>
                      </packing>
                    </child>
                    <child>
                      <object class="GtkEntry" id="main-search-entry">
                        <property name="visible">True</property>
                        <property name="can_focus">True</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="secondary_icon_stock">gtk-clear</property>
                        <property name="secondary_icon_tooltip_text">Clear the search</property>
                        <signal name="changed" handler="gva_main_search_entry_changed_cb" swapped="no"/>
                        <signal name="notify" handler="gva_main_search_entry_notify_cb" swapped="no"/>
                        <signal name="activate" handler="gva_main_search_entry_activate_cb" swapped="no"/>
                        <signal name="remove-widget" handler="gtk_widget_hide" object="main-search-hbox" swapped="yes"/>
                        <signal name="icon-release" handler="gva_main_search_entry_icon_release_cb" swapped="no"/>
                      </object>
                      <packing>
                        <property name="expand">True</property>
                        <property name="fill">True</property>
                        <property name="position">1</property>
                      </packing>
                    </child>
                  </object>
                  <packing>
                    <property name="expand">False</property>
                    <property name="fill">False</property>
                    <property name="position">1</property>
                  </packing>
                </child>
              </object>
              <packing>
                <property name="expand">True</property>
                <property name="fill">True</property>
                <property name="position">1</property>
              </packing>
            </child>
            <child>
              <object class="GtkHButtonBox" id="main-bottom-button-box">
                <property name="visible">True</property>
                <property name="can_focus">False</property>
                <property name="spacing">12</property>
                <property name="layout_style">end</property>
                <child>
                  <object class="GtkButton" id="main-properties-button">
                    <property name="visible">True</property>
                    <property name="can_focus">True</property>
                    <property name="can_default">True</property>
                    <property name="receives_default">False</property>
                    <property name="tooltip_text" translatable="yes">Show information about the selected game</property>
                    <property name="related_action">properties</property>
                    <property name="use_stock">True</property>
                  </object>
                  <packing>
                    <property name="expand">False</property>
                    <property name="fill">False</property>
                    <property name="position">1</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkButton" id="main-start-game-button">
                    <property name="label" translatable="yes">_Start Game</property>
                    <property name="visible">True</property>
                    <property name="can_focus">True</property>
                    <property name="has_focus">True</property>
                    <property name="can_default">True</property>
                    <property name="has_default">True</property>
                    <property name="receives_default">False</property>
                    <property name="tooltip_text" translatable="yes">Start the selected game</property>
                    <property name="related_action">start</property>
                    <property name="use_action_appearance">False</property>
                    <property name="image">main-start-game-image</property>
                    <property name="use_underline">True</property>
                  </object>
                  <packing>
                    <property name="expand">False</property>
                    <property name="fill">False</property>
                    <property name="position">2</property>
                  </packing>
                </child>
              </object>
              <packing>
                <property name="expand">False</property>
                <property name="fill">True</property>
                <property name="position">2</property>
              </packing>
            </child>
          </object>
          <packing>
            <property name="expand">True</property>
            <property name="fill">True</property>
            <property name="position">0</property>
          </packing>
        </child>
        <child>
          <object class="GtkHBox" id="main-hbox">
            <property name="visible">True</property>
            <property name="can_focus">False</property>
            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
            <child>
              <object class="GtkStatusbar" id="main-statusbar">
                <property name="visible">True</property>
                <property name="can_focus">False</property>
              </object>
              <packing>
                <property name="expand">True</property>
                <property name="fill">True</property>
                <property name="position">0</property>
              </packing>
            </child>
            <child>
              <object class="GtkProgressBar" id="main-progress-bar">
                <property name="can_focus">False</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
              </object>
              <packing>
                <property name="expand">False</property>
                <property name="fill">False</property>
                <property name="position">1</property>
              </packing>
            </child>
          </object>
          <packing>
            <property name="expand">False</property>
            <property name="fill">False</property>
            <property name="position">1</property>
          </packing>
        </child>
      </object>
    </child>
  </object>
  <object class="GtkImage" id="play-back-button-image">
    <property name="visible">True</property>
    <property name="can_focus">False</property>
    <property name="stock">gtk-media-play</property>
  </object>
  <object class="GtkWindow" id="play-back-window">
    <property name="can_focus">False</property>
    <property name="title" translatable="yes">Recorded Games</property>
    <property name="window_position">center-on-parent</property>
    <property name="default_width">550</property>
    <property name="default_height">250</property>
    <property name="destroy_with_parent">True</property>
    <property name="transient_for">main-window</property>
    <signal name="delete-event" handler="gtk_widget_hide_on_delete" swapped="no"/>
    <signal name="hide" handler="gva_play_back_window_hide_cb" swapped="no"/>
    <child>
      <object class="GtkVBox" id="play-back-vbox">
        <property name="visible">True</property>
        <property name="can_focus">False</property>
        <property name="border_width">12</property>
        <property name="spacing">12</property>
        <child>
          <object class="GtkScrolledWindow" id="play-back-scrolled-window">
            <property name="visible">True</property>
            <property name="can_focus">True</property>
            <property name="shadow_type">in</property>
            <child>
              <object class="GtkTreeView" id="play-back-tree-view">
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="reorderable">True</property>
                <property name="rules_hint">True</property>
                <property name="show_expanders">False</property>
                <signal name="row-activated" handler="gva_play_back_row_activated_cb" swapped="no"/>
                <child internal-child="selection">
                  <object class="GtkTreeSelection" id="treeview-selection2"/>
                </child>
              </object>
            </child>
          </object>
          <packing>
            <property name="expand">True</property>
            <property name="fill">True</property>
            <property name="position">0</property>
          </packing>
        </child>
        <child>
          <object class="GtkHButtonBox" id="play-back-button-box">
            <property name="visible">True</property>
            <property name="can_focus">False</property>
            <property name="spacing">12</property>
            <property name="layout_style">end</property>
            <child>
              <object class="GtkButton" id="play-back-close-button">
                <property name="label">gtk-close</property>
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="can_default">True</property>
                <property name="receives_default">False</property>
                <property name="tooltip_text" translatable="yes">Close this window</property>
                <property name="use_action_appearance">False</property>
                <property name="use_stock">True</property>
                <signal name="clicked" handler="gva_play_back_close_clicked_cb" object="play-back-window" swapped="yes"/>
              </object>
              <packing>
                <property name="expand">False</property>
                <property name="fill">False</property>
                <property name="position">0</property>
              </packing>
            </child>
            <child>
              <object class="GtkButton" id="play-back-delete-button">
                <property name="label">gtk-delete</property>
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="can_default">True</property>
                <property name="receives_default">False</property>
                <property name="tooltip_text" translatable="yes">Delete the selected game recordings</property>
                <property name="use_action_appearance">False</property>
                <property name="use_stock">True</property>
                <signal name="clicked" handler="gva_play_back_delete_clicked_cb" object="play-back-tree-view" swapped="yes"/>
              </object>
              <packing>
                <property name="expand">False</property>
                <property name="fill">False</property>
                <property name="position">1</property>
              </packing>
            </child>
            <child>
              <object class="GtkButton" id="play-back-button">
                <property name="label" translatable="yes">Play _Back</property>
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="has_focus">True</property>
                <property name="is_focus">True</property>
                <property name="can_default">True</property>
                <property name="has_default">True</property>
                <property name="receives_default">True</property>
                <property name="tooltip_text" translatable="yes">Play back the selected game recording</property>
                <property name="use_action_appearance">False</property>
                <property name="image">play-back-button-image</property>
                <property name="use_underline">True</property>
                <signal name="clicked" handler="gva_play_back_clicked_cb" swapped="no"/>
              </object>
              <packing>
                <property name="expand">False</property>
                <property name="fill">False</property>
                <property name="position">2</property>
              </packing>
            </child>
          </object>
          <packing>
            <property name="expand">False</property>
            <property name="fill">True</property>
            <property name="position">1</property>
          </packing>
        </child>
      </object>
    </child>
  </object>
  <object class="GtkWindow" id="preferences-window">
    <property name="can_focus">False</property>
    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
    <property name="title" translatable="yes">Preferences</property>
    <property name="window_position">center-on-parent</property>
    <property name="destroy_with_parent">True</property>
    <property name="transient_for">main-window</property>
    <signal name="delete-event" handler="gtk_widget_hide_on_delete" swapped="no"/>
    <child>
      <object class="GtkVBox" id="preferences-vbox">
        <property name="visible">True</property>
        <property name="can_focus">False</property>
        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
        <property name="border_width">12</property>
        <property name="spacing">12</property>
        <child>
          <object class="GtkNotebook" id="preferences-notebook">
            <property name="visible">True</property>
            <property name="can_focus">True</property>
            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
            <child>
              <object class="GtkVBox" id="preferences-general-tab-vbox">
                <property name="visible">True</property>
                <property name="can_focus">False</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="border_width">12</property>
                <property name="spacing">12</property>
                <child>
                  <object class="GtkVBox" id="preferences-general-vbox">
                    <property name="visible">True</property>
                    <property name="can_focus">False</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="spacing">6</property>
                    <child>
                      <object class="GtkLabel" id="preferences-general-label">
                        <property name="visible">True</property>
                        <property name="can_focus">False</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="xalign">0</property>
                        <property name="yalign">0</property>
                        <property name="label" translatable="yes">&lt;b&gt;General&lt;/b&gt;</property>
                        <property name="use_markup">True</property>
                      </object>
                      <packing>
                        <property name="expand">False</property>
                        <property name="fill">True</property>
                        <property name="position">0</property>
                      </packing>
                    </child>
                    <child>
                      <object class="GtkAlignment" id="preferences-general-alignment">
                        <property name="visible">True</property>
                        <property name="can_focus">False</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="left_padding">12</property>
                        <child>
                          <object class="GtkVBox" id="preferences-general-inner-vbox">
                            <property name="visible">True</property>
                            <property name="can_focus">False</property>
                            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkCheckButton" id="preferences-full-screen">
                                <property name="visible">True</property>
                                <property name="can_focus">True</property>
                                <property name="receives_default">False</property>
                                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                                <property name="related_action">full-screen</property>
                                <property name="use_underline">True</property>
                                <property name="xalign">0</property>
                                <property name="draw_indicator">True</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="fill">True</property>
                                <property name="position">0</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkCheckButton" id="preferences-auto-save">
                                <property name="visible">True</property>
                                <property name="can_focus">True</property>
                                <property name="receives_default">False</property>
                                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                                <property name="related_action">auto-save</property>
                                <property name="use_underline">True</property>
                                <property name="xalign">0</property>
                                <property name="draw_indicator">True</property>
                              </object>
                              <packing>
                                <property name="expand">True</property>
                                <property name="fill">True</property>
                                <property name="position">1</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkAlignment" id="preferences-auto-save-note-alignment">
                                <property name="visible">True</property>
                                <property name="can_focus">False</property>
                                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                                <property name="left_padding">24</property>
                                <child>
                                  <object class="GtkLabel" id="preferences-auto-save-info-label">
                                    <property name="visible">True</property>
                                    <property name="can_focus">False</property>
                                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                                    <property name="xalign">0</property>
                                    <property name="yalign">0</property>
                                    <property name="label" translatable="yes">&lt;small&gt;&lt;i&gt;&lt;b&gt;Note:&lt;/b&gt; Setting this will not take effect until the second time the game is started. It does not apply to recording or playing back games.&lt;/i&gt;&lt;/small&gt;</property>
                                    <property name="use_markup">True</property>
                                    <property name="wrap">True</property>
                                  </object>
                                </child>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="fill">False</property>
                                <property name="position">2</property>
                              </packing>
                            </child>
                          </object>
                        </child>
                      </object>
                      <packing>
                        <property name="expand">False</property>
                        <property name="fill">False</property>
                        <property name="position">1</property>
                      </packing>
                    </child>
                  </object>
                  <packing>
                    <property name="expand">False</property>
                    <property name="fill">False</property>
                    <property name="position">0</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkVBox" id="preferences-game-list-vbox">
                    <property name="visible">True</property>
                    <property name="can_focus">False</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="spacing">6</property>
                    <child>
                      <object class="GtkLabel" id="preferences-game-list-label">
                        <property name="visible">True</property>
                        <property name="can_focus">False</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="xalign">0</property>
                        <property name="label" translatable="yes">&lt;b&gt;Game List&lt;/b&gt;</property>
                        <property name="use_markup">True</property>
                      </object>
                      <packing>
                        <property name="expand">False</property>
                        <property name="fill">True</property>
                        <property name="position">0</property>
                      </packing>
                    </child>
                    <child>
                      <object class="GtkAlignment" id="preferences-game-list-alignment">
                        <property name="visible">True</property>
                        <property name="can_focus">False</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="left_padding">12</property>
                        <child>
                          <object class="GtkVBox" id="preferences-game-list-inner-vbox">
                            <property name="visible">True</property>
                            <property name="can_focus">False</property>
                            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkCheckButton" id="preferences-show-clones">
                                <property name="visible">True</property>
                                <property name="can_focus">True</property>
                                <property name="receives_default">False</property>
                                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                                <property name="related_action">show-clones</property>
                                <property name="xalign">0</property>
                                <property name="draw_indicator">True</property>
                              </object>
                              <packing>
                                <property name="expand">True</property>
                                <property name="fill">True</property>
                                <property name="position">0</property>
                              </packing>
                            </child>
                          </object>
                        </child>
                      </object>
                      <packing>
                        <property name="expand">True</property>
                        <property name="fill">True</property>
                        <property name="position">1</property>
                      </packing>
                    </child>
                  </object>
                  <packing>
                    <property name="expand">False</property>
                    <property name="fill">False</property>
                    <property name="position">1</property>
                  </packing>
                </child>
              </object>
            </child>
            <child type="tab">
              <object class="GtkLabel" id="preferences-general-tab-label">
                <property name="visible">True</property>
                <property name="can_focus">False</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="label" translatable="yes">General</property>
              </object>
              <packing>
                <property name="tab_fill">False</property>
              </packing>
            </child>
            <child>
              <object class="GtkVBox" id="preferences-columns-tab-vbox">
                <property name="visible">True</property>
                <property name="can_focus">False</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="border_width">12</property>
                <property name="spacing">6</property>
                <child>
                  <object class="GtkLabel" id="preferences-columns-label">
                    <property name="visible">True</property>
                    <property name="can_focus">False</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="xalign">0</property>
                    <property name="label" translatable="yes">&lt;b&gt;Columns&lt;/b&gt;</property>
                    <property name="use_markup">True</property>
                  </object>
                  <packing>
                    <property name="expand">False</property>
                    <property name="fill">False</property>
                    <property name="position">0</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkAlignment" id="preferences-columns-alignment">
                    <property name="visible">True</property>
                    <property name="can_focus">False</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="left_padding">12</property>
                    <child>
                      <object class="GtkVBox" id="preferences-columns-inner-vbox">
                        <property name="visible">True</property>
                        <property name="can_focus">False</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="spacing">6</property>
                        <child>
                          <object class="GtkLabel" id="preferences-columns-instructions1">
                            <property name="visible">True</property>
                            <property name="can_focus">False</property>
                            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">Choose the order of information to appear in the game list.</property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GvaColumnManager" id="preferences-column-manager">
                            <property name="visible">True</property>
                            <property name="can_focus">False</property>
                            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                            <property name="spacing">6</property>
                            <property name="managed_view">main-tree-view</property>
                          </object>
                          <packing>
                            <property name="expand">True</property>
                            <property name="fill">True</property>
                            <property name="position">1</property>
                          </packing>
                        </child>
                      </object>
                    </child>
                  </object>
                  <packing>
                    <property name="expand">True</property>
                    <property name="fill">True</property>
                    <property name="position">1</property>
                  </packing>
                </child>
              </object>
              <packing>
                <property name="position">1</property>
              </packing>
            </child>
            <child type="tab">
              <object class="GtkLabel" id="preferences-columns-tab-label">
                <property name="visible">True</property>
                <property name="can_focus">False</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="label" translatable="yes">Columns</property>
              </object>
              <packing>
                <property name="position">1</property>
                <property name="tab_fill">False</property>
              </packing>
            </child>
          </object>
          <packing>
            <property name="expand">True</property>
            <property name="fill">True</property>
            <property name="position">0</property>
          </packing>
        </child>
        <child>
          <object class="GtkHButtonBox" id="preferences-button-box">
            <property name="visible">True</property>
            <property name="can_focus">False</property>
            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
            <property name="spacing">12</property>
            <property name="layout_style">end</property>
            <child>
              <object class="GtkButton" id="preferences-close-button">
                <property name="label">gtk-close</property>
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="has_focus">True</property>
                <property name="is_focus">True</property>
                <property name="can_default">True</property>
                <property name="has_default">True</property>
                <property name="receives_default">True</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="tooltip_text" translatable="yes">Close this window</property>
                <property name="use_action_appearance">False</property>
                <property name="use_stock">True</property>
                <signal name="clicked" handler="gva_preferences_close_clicked_cb" object="preferences-window" swapped="yes"/>
              </object>
              <packing>
                <property name="expand">False</property>
                <property name="fill">False</property>
                <property name="position">0</property>
              </packing>
            </child>
          </object>
          <packing>
            <property name="expand">False</property>
            <property name="fill">True</property>
            <property name="position">1</property>
          </packing>
        </child>
      </object>
    </child>
  </object>
  <object class="GtkWindow" id="properties-window">
    <property name="can_focus">False</property>
    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
    <property name="border_width">12</property>
    <property name="title" translatable="yes">Properties</property>
    <property name="window_position">center-on-parent</property>
    <property name="default_width">500</property>
    <property name="default_height">400</property>
    <property name="destroy_with_parent">True</property>
    <property name="transient_for">main-window</property>
    <signal name="show" handler="gva_properties_show_cb" swapped="no"/>
    <signal name="configure-event" handler="gva_properties_configure_event_cb" swapped="no"/>
    <signal name="delete-event" handler="gtk_widget_hide_on_delete" swapped="no"/>
    <signal name="hide" handler="gva_properties_hide_cb" swapped="no"/>
    <signal name="window-state-event" handler="gva_properties_window_state_event_cb" swapped="no"/>
    <child>
      <object class="GtkVBox" id="properties-vbox">
        <property name="visible">True</property>
        <property name="can_focus">False</property>
        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
        <property name="spacing">12</property>
        <child>
          <object class="GtkTable" id="properties-table">
            <property name="visible">True</property>
            <property name="can_focus">False</property>
            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
            <property name="n_rows">2</property>
            <property name="n_columns">2</property>
            <property name="column_spacing">12</property>
            <child>
              <object class="GtkHBox" id="properties-upper-hbox">
                <property name="visible">True</property>
                <property name="can_focus">False</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="spacing">3</property>
                <property name="homogeneous">True</property>
                <child>
                  <object class="GtkButton" id="properties-back-button">
                    <property name="visible">True</property>
                    <property name="can_focus">True</property>
                    <property name="receives_default">True</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="tooltip_text" translatable="yes">Show previous game</property>
                    <property name="related_action">previous-game</property>
                    <property name="use_action_appearance">False</property>
                    <child>
                      <object class="GtkImage" id="properties-back-button-image">
                        <property name="visible">True</property>
                        <property name="can_focus">False</property>
                        <property name="stock">gtk-go-back</property>
                      </object>
                    </child>
                  </object>
                  <packing>
                    <property name="expand">False</property>
                    <property name="fill">False</property>
                    <property name="position">0</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkButton" id="properties-forward-button">
                    <property name="visible">True</property>
                    <property name="can_focus">True</property>
                    <property name="receives_default">True</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="tooltip_text" translatable="yes">Show next game</property>
                    <property name="related_action">next-game</property>
                    <property name="use_action_appearance">False</property>
                    <child>
                      <object class="GtkImage" id="properties-forward-button-image">
                        <property name="visible">True</property>
                        <property name="can_focus">False</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="stock">gtk-go-forward</property>
                      </object>
                    </child>
                  </object>
                  <packing>
                    <property name="expand">False</property>
                    <property name="fill">False</property>
                    <property name="position">1</property>
                  </packing>
                </child>
              </object>
              <packing>
                <property name="left_attach">1</property>
                <property name="right_attach">2</property>
                <property name="x_options"></property>
                <property name="y_options"></property>
              </packing>
            </child>
            <child>
              <object class="GtkLabel" id="properties-header">
                <property name="visible">True</property>
                <property name="can_focus">False</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="xalign">0</property>
                <property name="yalign">0</property>
                <property name="label">&lt;big&gt;&lt;b&gt;Game Description&lt;/b&gt;&lt;/big&gt;
Manufacturer, Year</property>
                <property name="use_markup">True</property>
                <property name="ellipsize">end</property>
              </object>
              <packing>
                <property name="bottom_attach">2</property>
                <property name="y_options">GTK_FILL</property>
              </packing>
            </child>
            <child>
              <placeholder/>
            </child>
          </object>
          <packing>
            <property name="expand">False</property>
            <property name="fill">False</property>
            <property name="position">0</property>
          </packing>
        </child>
        <child>
          <object class="GtkNotebook" id="properties-notebook">
            <property name="visible">True</property>
            <property name="can_focus">True</property>
            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
            <child>
              <object class="GtkNotebook" id="properties-history-notebook">
                <property name="visible">True</property>
                <property name="can_focus">False</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="show_tabs">False</property>
                <property name="show_border">False</property>
                <child>
                  <object class="GtkScrolledWindow" id="properties-history-scrolled-window">
                    <property name="visible">True</property>
                    <property name="can_focus">True</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="border_width">6</property>
                    <property name="shadow_type">in</property>
                    <child>
                      <object class="GtkTextView" id="properties-history-text-view">
                        <property name="visible">True</property>
                        <property name="can_focus">True</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="editable">False</property>
                        <property name="wrap_mode">word</property>
                        <property name="left_margin">6</property>
                        <property name="right_margin">6</property>
                        <property name="cursor_visible">False</property>
                        <property name="accepts_tab">False</property>
                      </object>
                    </child>
                  </object>
                </child>
                <child type="tab">
                  <placeholder/>
                </child>
                <child>
                  <placeholder/>
                </child>
                <child type="tab">
                  <placeholder/>
                </child>
              </object>
            </child>
            <child type="tab">
              <object class="GtkLabel" id="properties-history-tab">
                <property name="visible">True</property>
                <property name="can_focus">False</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="label" translatable="yes">History</property>
              </object>
              <packing>
                <property name="tab_fill">False</property>
              </packing>
            </child>
            <child>
              <object class="GtkNotebook" id="properties-gallery-notebook">
                <property name="visible">True</property>
                <property name="can_focus">False</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="show_tabs">False</property>
                <property name="show_border">False</property>
                <child>
                  <object class="GtkImage" id="properties-gallery-image">
                    <property name="visible">True</property>
                    <property name="can_focus">False</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="xpad">6</property>
                    <property name="ypad">6</property>
                    <property name="stock">gtk-missing-image</property>
                  </object>
                </child>
                <child type="tab">
                  <placeholder/>
                </child>
                <child>
                  <placeholder/>
                </child>
                <child type="tab">
                  <placeholder/>
                </child>
              </object>
              <packing>
                <property name="position">1</property>
              </packing>
            </child>
            <child type="tab">
              <object class="GtkLabel" id="properties-gallery-tab">
                <property name="visible">True</property>
                <property name="can_focus">False</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="label" translatable="yes">Gallery</property>
              </object>
              <packing>
                <property name="position">1</property>
                <property name="tab_fill">False</property>
              </packing>
            </child>
            <child>
              <object class="GtkScrolledWindow" id="properties-technical-scrolled-window">
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="border_width">6</property>
                <property name="hscrollbar_policy">never</property>
                <child>
                  <object class="GtkViewport" id="properties-technical-viewport">
                    <property name="visible">True</property>
                    <property name="can_focus">True</property>
                    <property name="resize_mode">queue</property>
                    <child>
                      <object class="GtkVBox" id="properties-technical-vbox">
                        <property name="visible">True</property>
                        <property name="can_focus">False</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="border_width">12</property>
                        <property name="spacing">12</property>
                        <child>
                          <object class="GtkFrame" id="properties-status-frame">
                            <property name="visible">True</property>
                            <property name="can_focus">False</property>
                            <property name="label_xalign">0</property>
                            <property name="label_yalign">0</property>
                            <property name="shadow_type">out</property>
                            <child>
                              <object class="GtkAlignment" id="properties-status-frame-alignment">
                                <property name="visible">True</property>
                                <property name="can_focus">False</property>
                                <child>
                                  <object class="GtkEventBox" id="properties-status-event-box">
                                    <property name="visible">True</property>
                                    <property name="can_focus">False</property>
                                    <child>
                                      <object class="GtkHBox" id="properties-status-hbox">
                                        <property name="visible">True</property>
                                        <property name="can_focus">False</property>
                                        <property name="border_width">6</property>
                                        <property name="spacing">12</property>
                                        <child>
                                          <object class="GtkImage" id="properties-status-image">
                                            <property name="visible">True</property>
                                            <property name="can_focus">False</property>
                                            <property name="yalign">0</property>
                                            <property name="stock">gtk-dialog-warning</property>
                                            <property name="icon-size">5</property>
                                          </object>
                                          <packing>
                                            <property name="expand">False</property>
                                            <property name="fill">False</property>
                                            <property name="position">0</property>
                                          </packing>
                                        </child>
                                        <child>
                                          <object class="GtkVBox" id="properties-status-vbox">
                                            <property name="visible">True</property>
                                            <property name="can_focus">False</property>
                                            <property name="spacing">6</property>
                                            <child>
                                              <object class="GtkLabel" id="properties-status-header">
                                                <property name="visible">True</property>
                                                <property name="can_focus">False</property>
                                                <property name="xalign">0</property>
                                                <property name="label" translatable="yes">&lt;b&gt;There are known problems with this game:&lt;/b&gt;</property>
                                                <property name="use_markup">True</property>
                                                <property name="ellipsize">end</property>
                                              </object>
                                              <packing>
                                                <property name="expand">False</property>
                                                <property name="fill">False</property>
                                                <property name="position">0</property>
                                              </packing>
                                            </child>
                                            <child>
                                              <object class="GtkVBox" id="properties-status-detail-vbox">
                                                <property name="visible">True</property>
                                                <property name="can_focus">False</property>
                                                <child>
                                                  <object class="GtkLabel" id="properties-imperfect-color-label">
                                                    <property name="visible">True</property>
                                                    <property name="can_focus">False</property>
                                                    <property name="xalign">0</property>
                                                    <property name="label" translatable="yes">• The colors aren't 100% accurate.</property>
                                                    <property name="ellipsize">end</property>
                                                  </object>
                                                  <packing>
                                                    <property name="expand">True</property>
                                                    <property name="fill">True</property>
                                                    <property name="position">0</property>
                                                  </packing>
                                                </child>
                                                <child>
                                                  <object class="GtkLabel" id="properties-preliminary-color-label">
                                                    <property name="visible">True</property>
                                                    <property name="can_focus">False</property>
                                                    <property name="xalign">0</property>
                                                    <property name="label" translatable="yes">• The colors are completely wrong.</property>
                                                    <property name="ellipsize">end</property>
                                                  </object>
                                                  <packing>
                                                    <property name="expand">True</property>
                                                    <property name="fill">True</property>
                                                    <property name="position">1</property>
                                                  </packing>
                                                </child>
                                                <child>
                                                  <object class="GtkLabel" id="properties-imperfect-graphic-label">
                                                    <property name="visible">True</property>
                                                    <property name="can_focus">False</property>
                                                    <property name="xalign">0</property>
                                                    <property name="label" translatable="yes">• The video emulation isn't 100% accurate.</property>
                                                    <property name="ellipsize">end</property>
                                                  </object>
                                                  <packing>
                                                    <property name="expand">True</property>
                                                    <property name="fill">True</property>
                                                    <property name="position">2</property>
                                                  </packing>
                                                </child>
                                                <child>
                                                  <object class="GtkLabel" id="properties-imperfect-sound-label">
                                                    <property name="visible">True</property>
                                                    <property name="can_focus">False</property>
                                                    <property name="xalign">0</property>
                                                    <property name="label" translatable="yes">• The sound emulation isn't 100% accurate.</property>
                                                    <property name="ellipsize">end</property>
                                                  </object>
                                                  <packing>
                                                    <property name="expand">True</property>
                                                    <property name="fill">True</property>
                                                    <property name="position">3</property>
                                                  </packing>
                                                </child>
                                                <child>
                                                  <object class="GtkLabel" id="properties-preliminary-sound-label">
                                                    <property name="visible">True</property>
                                                    <property name="can_focus">False</property>
                                                    <property name="xalign">0</property>
                                                    <property name="label" translatable="yes">• The game lacks sound.</property>
                                                    <property name="ellipsize">end</property>
                                                  </object>
                                                  <packing>
                                                    <property name="expand">True</property>
                                                    <property name="fill">True</property>
                                                    <property name="position">4</property>
                                                  </packing>
                                                </child>
                                                <child>
                                                  <object class="GtkLabel" id="properties-preliminary-cocktail-label">
                                                    <property name="visible">True</property>
                                                    <property name="can_focus">False</property>
                                                    <property name="xalign">0</property>
                                                    <property name="label" translatable="yes">• Screen flipping in cocktail mode is not supported.</property>
                                                    <property name="ellipsize">end</property>
                                                  </object>
                                                  <packing>
                                                    <property name="expand">True</property>
                                                    <property name="fill">True</property>
                                                    <property name="position">5</property>
                                                  </packing>
                                                </child>
                                                <child>
                                                  <object class="GtkLabel" id="properties-preliminary-emulation-label">
                                                    <property name="visible">True</property>
                                                    <property name="can_focus">False</property>
                                                    <property name="xalign">0</property>
                                                    <property name="label" translatable="yes">• &lt;b&gt;THIS GAME DOESN'T WORK.&lt;/b&gt;</property>
                                                    <property name="use_markup">True</property>
                                                  </object>
                                                  <packing>
                                                    <property name="expand">False</property>
                                                    <property name="fill">False</property>
                                                    <property name="position">6</property>
                                                  </packing>
                                                </child>
                                                <child>
                                                  <object class="GtkLabel" id="properties-preliminary-protection-label">
                                                    <property name="visible">True</property>
                                                    <property name="can_focus">False</property>
                                                    <property name="xalign">0</property>
                                                    <property name="label" translatable="yes">• The game has protection which isn't fully emulated.</property>
                                                  </object>
                                                  <packing>
                                                    <property name="expand">False</property>
                                                    <property name="fill">False</property>
                                                    <property name="position">7</property>
                                                  </packing>
                                                </child>
                                              </object>
                                              <packing>
                                                <property name="expand">False</property>
                                                <property name="fill">False</property>
                                                <property name="position">1</property>
                                              </packing>
                                            </child>
                                          </object>
                                          <packing>
                                            <property name="expand">True</property>
                                            <property name="fill">True</property>
                                            <property name="position">1</property>
                                          </packing>
                                        </child>
                                      </object>
                                    </child>
                                  </object>
                                </child>
                              </object>
                            </child>
                            <child type="label">
                              <object class="GtkLabel" id="properties-status-frame-label">
                                <property name="can_focus">False</property>
                              </object>
                            </child>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkVBox" id="properties-bios-vbox">
                            <property name="visible">True</property>
                            <property name="can_focus">False</property>
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkLabel" id="properties-bios-header">
                                <property name="visible">True</property>
                                <property name="can_focus">False</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">&lt;b&gt;BIOS&lt;/b&gt;</property>
                                <property name="use_markup">True</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="fill">False</property>
                                <property name="position">0</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkAlignment" id="properties-bios-alignment">
                                <property name="visible">True</property>
                                <property name="can_focus">False</property>
                                <property name="left_padding">12</property>
                                <child>
                                  <object class="GtkLabel" id="properties-bios-label">
                                    <property name="visible">True</property>
                                    <property name="can_focus">False</property>
                                    <property name="xalign">0</property>
                                    <property name="label">BIOS System Name</property>
                                  </object>
                                </child>
                              </object>
                              <packing>
                                <property name="expand">True</property>
                                <property name="fill">True</property>
                                <property name="position">1</property>
                              </packing>
                            </child>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">1</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkVBox" id="properties-cpu-vbox">
                            <property name="visible">True</property>
                            <property name="can_focus">False</property>
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkLabel" id="properties-cpu-header">
                                <property name="visible">True</property>
                                <property name="can_focus">False</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">&lt;b&gt;CPU&lt;/b&gt;</property>
                                <property name="use_markup">True</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="fill">False</property>
                                <property name="position">0</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkAlignment" id="properties-cpu-alignment">
                                <property name="visible">True</property>
                                <property name="can_focus">False</property>
                                <property name="left_padding">12</property>
                                <child>
                                  <object class="GtkVBox" id="properties-cpu-inner-vbox">
                                    <property name="visible">True</property>
                                    <property name="can_focus">False</property>
                                    <property name="spacing">6</property>
                                    <child>
                                      <object class="GtkLabel" id="properties-cpu0-label">
                                        <property name="visible">True</property>
                                        <property name="can_focus">False</property>
                                        <property name="xalign">0</property>
                                        <property name="label">CPU #0 Information</property>
                                      </object>
                                      <packing>
                                        <property name="expand">False</property>
                                        <property name="fill">False</property>
                                        <property name="position">0</property>
                                      </packing>
                                    </child>
                                    <child>
                                      <object class="GtkLabel" id="properties-cpu1-label">
                                        <property name="visible">True</property>
                                        <property name="can_focus">False</property>
                                        <property name="xalign">0</property>
                                        <property name="label">CPU #1 Information</property>
                                      </object>
                                      <packing>
                                        <property name="expand">False</property>
                                        <property name="fill">False</property>
                                        <property name="position">1</property>
                                      </packing>
                                    </child>
                                    <child>
                                      <object class="GtkLabel" id="properties-cpu2-label">
                                        <property name="visible">True</property>
                                        <property name="can_focus">False</property>
                                        <property name="xalign">0</property>
                                        <property name="label">CPU #2 Information</property>
                                      </object>
                                      <packing>
                                        <property name="expand">False</property>
                                        <property name="fill">False</property>
                                        <property name="position">2</property>
                                      </packing>
                                    </child>
                                    <child>
                                      <object class="GtkLabel" id="properties-cpu3-label">
                                        <property name="visible">True</property>
                                        <property name="can_focus">False</property>
                                        <property name="xalign">0</property>
                                        <property name="label">CPU #3 Information</property>
                                      </object>
                                      <packing>
                                        <property name="expand">False</property>
                                        <property name="fill">False</property>
                                        <property name="position">3</property>
                                      </packing>
                                    </child>
                                  </object>
                                </child>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="fill">False</property>
                                <property name="position">1</property>
                              </packing>
                            </child>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">2</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkVBox" id="properties-sound-vbox">
                            <property name="visible">True</property>
                            <property name="can_focus">False</property>
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkLabel" id="properties-sound-header">
                                <property name="visible">True</property>
                                <property name="can_focus">False</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">&lt;b&gt;Sound&lt;/b&gt;</property>
                                <property name="use_markup">True</property>
                              </object>
                              <packing>
                                <property name="expand">True</property>
                                <property name="fill">True</property>
                                <property name="position">0</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkAlignment" id="properties-sound-alignment">
                                <property name="visible">True</property>
                                <property name="can_focus">False</property>
                                <property name="left_padding">12</property>
                                <child>
                                  <object class="GtkVBox" id="properties-sound-inner-vbox">
                                    <property name="visible">True</property>
                                    <property name="can_focus">False</property>
                                    <property name="spacing">6</property>
                                    <child>
                                      <object class="GtkLabel" id="properties-sound0-label">
                                        <property name="visible">True</property>
                                        <property name="can_focus">False</property>
                                        <property name="xalign">0</property>
                                        <property name="label">Sound #0 Information</property>
                                      </object>
                                      <packing>
                                        <property name="expand">False</property>
                                        <property name="fill">False</property>
                                        <property name="position">0</property>
                                      </packing>
                                    </child>
                                    <child>
                                      <object class="GtkLabel" id="properties-sound1-label">
                                        <property name="visible">True</property>
                                        <property name="can_focus">False</property>
                                        <property name="xalign">0</property>
                                        <property name="label">Sound #1 Information</property>
                                      </object>
                                      <packing>
                                        <property name="expand">False</property>
                                        <property name="fill">False</property>
                                        <property name="position">1</property>
                                      </packing>
                                    </child>
                                    <child>
                                      <object class="GtkLabel" id="properties-sound2-label">
                                        <property name="visible">True</property>
                                        <property name="can_focus">False</property>
                                        <property name="xalign">0</property>
                                        <property name="label">Sound #2 Information</property>
                                      </object>
                                      <packing>
                                        <property name="expand">False</property>
                                        <property name="fill">False</property>
                                        <property name="position">2</property>
                                      </packing>
                                    </child>
                                    <child>
                                      <object class="GtkLabel" id="properties-sound3-label">
                                        <property name="visible">True</property>
                                        <property name="can_focus">False</property>
                                        <property name="xalign">0</property>
                                        <property name="label">Sound #3 Information</property>
                                      </object>
                                      <packing>
                                        <property name="expand">False</property>
                                        <property name="fill">False</property>
                                        <property name="position">3</property>
                                      </packing>
                                    </child>
                                  </object>
                                </child>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="fill">False</property>
                                <property name="position">1</property>
                              </packing>
                            </child>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">3</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkVBox" id="properties-video-vbox">
                            <property name="visible">True</property>
                            <property name="can_focus">False</property>
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkLabel" id="properties-video-header">
                                <property name="visible">True</property>
                                <property name="can_focus">False</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">&lt;b&gt;Video&lt;/b&gt;</property>
                                <property name="use_markup">True</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="fill">False</property>
                                <property name="position">0</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkAlignment" id="properties-video-alignment">
                                <property name="visible">True</property>
                                <property name="can_focus">False</property>
                                <property name="left_padding">12</property>
                                <child>
                                  <object class="GtkVBox" id="properties-video-inner-vbox">
                                    <property name="visible">True</property>
                                    <property name="can_focus">False</property>
                                    <property name="spacing">6</property>
                                    <child>
                                      <object class="GtkLabel" id="properties-video0-label">
                                        <property name="visible">True</property>
                                        <property name="can_focus">False</property>
                                        <property name="xalign">0</property>
                                        <property name="label">Video #0 Information</property>
                                      </object>
                                      <packing>
                                        <property name="expand">False</property>
                                        <property name="fill">False</property>
                                        <property name="position">0</property>
                                      </packing>
                                    </child>
                                    <child>
                                      <object class="GtkLabel" id="properties-video1-label">
                                        <property name="visible">True</property>
                                        <property name="can_focus">False</property>
                                        <property name="xalign">0</property>
                                        <property name="label">Video #1 Information</property>
                                      </object>
                                      <packing>
                                        <property name="expand">False</property>
                                        <property name="fill">False</property>
                                        <property name="position">1</property>
                                      </packing>
                                    </child>
                                    <child>
                                      <object class="GtkLabel" id="properties-video2-label">
                                        <property name="visible">True</property>
                                        <property name="can_focus">False</property>
                                        <property name="xalign">0</property>
                                        <property name="label">Video #2 Information</property>
                                      </object>
                                      <packing>
                                        <property name="expand">False</property>
                                        <property name="fill">False</property>
                                        <property name="position">2</property>
                                      </packing>
                                    </child>
                                    <child>
                                      <object class="GtkLabel" id="properties-video3-label">
                                        <property name="visible">True</property>
                                        <property name="can_focus">False</property>
                                        <property name="xalign">0</property>
                                        <property name="label">Video #3 Information</property>
                                      </object>
                                      <packing>
                                        <property name="expand">False</property>
                                        <property name="fill">False</property>
                                        <property name="position">3</property>
                                      </packing>
                                    </child>
                                  </object>
                                </child>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="fill">False</property>
                                <property name="position">1</property>
                              </packing>
                            </child>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">4</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkVBox" id="properties-original-vbox">
                            <property name="visible">True</property>
                            <property name="can_focus">False</property>
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkLabel" id="properties-original-header">
                                <property name="visible">True</property>
                                <property name="can_focus">False</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">&lt;b&gt;Original Version&lt;/b&gt;</property>
                                <property name="use_markup">True</property>
                              </object>
                              <packing>
                                <property name="expand">True</property>
                                <property name="fill">True</property>
                                <property name="position">0</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkAlignment" id="properties-original-alignment">
                                <property name="visible">True</property>
                                <property name="can_focus">False</property>
                                <property name="left_padding">12</property>
                                <child>
                                  <object class="GtkVBox" id="properties-original-links">
                                    <property name="visible">True</property>
                                    <property name="can_focus">False</property>
                                    <property name="spacing">6</property>
                                    <child>
                                      <placeholder/>
                                    </child>
                                  </object>
                                </child>
                              </object>
                              <packing>
                                <property name="expand">True</property>
                                <property name="fill">True</property>
                                <property name="position">1</property>
                              </packing>
                            </child>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">5</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkVBox" id="properties-alternate-vbox">
                            <property name="visible">True</property>
                            <property name="can_focus">False</property>
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkLabel" id="properties-alternate-header">
                                <property name="visible">True</property>
                                <property name="can_focus">False</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">&lt;b&gt;Alternate Versions&lt;/b&gt;</property>
                                <property name="use_markup">True</property>
                              </object>
                              <packing>
                                <property name="expand">True</property>
                                <property name="fill">True</property>
                                <property name="position">0</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkAlignment" id="properties-alternate-alignment">
                                <property name="visible">True</property>
                                <property name="can_focus">False</property>
                                <property name="left_padding">12</property>
                                <child>
                                  <object class="GtkVBox" id="properties-alternate-links">
                                    <property name="visible">True</property>
                                    <property name="can_focus">False</property>
                                    <property name="spacing">6</property>
                                    <child>
                                      <placeholder/>
                                    </child>
                                  </object>
                                </child>
                              </object>
                              <packing>
                                <property name="expand">True</property>
                                <property name="fill">True</property>
                                <property name="position">1</property>
                              </packing>
                            </child>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">6</property>
                          </packing>
                        </child>
                      </object>
                    </child>
                  </object>
                </child>
              </object>
              <packing>
                <property name="position">2</property>
              </packing>
            </child>
            <child type="tab">
              <object class="GtkLabel" id="properties-technical-tab">
                <property name="visible">True</property>
                <property name="can_focus">False</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="label" translatable="yes">Technical</property>
              </object>
              <packing>
                <property name="position">2</property>
                <property name="tab_fill">False</property>
              </packing>
            </child>
          </object>
          <packing>
            <property name="expand">True</property>
            <property name="fill">True</property>
            <property name="position">1</property>
          </packing>
        </child>
        <child>
          <object class="GtkHBox" id="properties-lower-hbox">
            <property name="visible">True</property>
            <property name="can_focus">False</property>
            <property name="spacing">12</property>
            <child>
              <object class="GtkTable" id="properties-music-table">
                <property name="visible">True</property>
                <property name="can_focus">False</property>
                <property name="n_rows">2</property>
                <property name="n_columns">2</property>
                <property name="column_spacing">6</property>
                <child>
                  <object class="GtkAlignment" id="properties-music-alignment">
                    <property name="visible">True</property>
                    <property name="can_focus">False</property>
                    <property name="yalign">1</property>
                    <property name="yscale">0</property>
                    <child>
                      <object class="GvaMusicButton" id="properties-music-button">
                        <property name="width_request">64</property>
                        <property name="visible">True</property>
                        <property name="can_focus">True</property>
                        <property name="receives_default">True</property>
                        <property name="tooltip_text" translatable="yes">Play a music clip</property>
                        <property name="use_action_appearance">False</property>
                      </object>
                    </child>
                  </object>
                  <packing>
                    <property name="bottom_attach">2</property>
                    <property name="x_options"></property>
                    <property name="y_options">GTK_FILL</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkLabel" id="properties-music-status">
                    <property name="visible">True</property>
                    <property name="can_focus">False</property>
                    <property name="xalign">0</property>
                    <property name="label" translatable="yes">&lt;small&gt;&lt;/small&gt;</property>
                    <property name="use_markup">True</property>
                    <property name="ellipsize">end</property>
                  </object>
                  <packing>
                    <property name="left_attach">1</property>
                    <property name="right_attach">2</property>
                    <property name="y_options">GTK_FILL</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkCheckButton" id="properties-music-auto-play">
                    <property name="visible">True</property>
                    <property name="can_focus">True</property>
                    <property name="receives_default">False</property>
                    <property name="related_action">auto-play</property>
                    <property name="use_underline">True</property>
                    <property name="xalign">0</property>
                    <property name="draw_indicator">True</property>
                  </object>
                  <packing>
                    <property name="left_attach">1</property>
                    <property name="right_attach">2</property>
                    <property name="top_attach">1</property>
                    <property name="bottom_attach">2</property>
                    <property name="y_options">GTK_FILL</property>
                  </packing>
                </child>
              </object>
              <packing>
                <property name="expand">True</property>
                <property name="fill">True</property>
                <property name="position">0</property>
              </packing>
            </child>
            <child>
              <object class="GtkAlignment" id="properties-button-box-alignment">
                <property name="visible">True</property>
                <property name="can_focus">False</property>
                <property name="yalign">1</property>
                <property name="yscale">0</property>
                <child>
                  <object class="GtkHButtonBox" id="properties-button-box">
                    <property name="visible">True</property>
                    <property name="can_focus">False</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="layout_style">end</property>
                    <child>
                      <object class="GtkButton" id="properties-close-button">
                        <property name="label">gtk-close</property>
                        <property name="visible">True</property>
                        <property name="can_focus">True</property>
                        <property name="can_default">True</property>
                        <property name="has_default">True</property>
                        <property name="receives_default">True</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="tooltip_text" translatable="yes">Close this window</property>
                        <property name="use_action_appearance">False</property>
                        <property name="use_stock">True</property>
                        <signal name="clicked" handler="gtk_widget_hide" object="properties-window" swapped="yes"/>
                      </object>
                      <packing>
                        <property name="expand">False</property>
                        <property name="fill">False</property>
                        <property name="position">0</property>
                      </packing>
                    </child>
                  </object>
                </child>
              </object>
              <packing>
                <property name="expand">False</property>
                <property name="fill">False</property>
                <property name="pack_type">end</property>
                <property name="position">1</property>
              </packing>
            </child>
          </object>
          <packing>
            <property name="expand">False</property>
            <property name="fill">False</property>
            <property name="position">2</property>
          </packing>
        </child>
      </object>
    </child>
  </object>
</interface>
