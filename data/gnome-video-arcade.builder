<?xml version="1.0"?>
<!--*- mode: xml -*-->
<interface>
  <object class="GtkWindow" id="main-window">
    <property name="title" translatable="yes">GNOME Video Arcade</property>
    <property name="default_width">500</property>
    <property name="default_height">400</property>
    <property name="icon_name">gnome-video-arcade</property>
    <signal handler="gva_main_window_destroy_cb" name="destroy"/>
    <child>
      <object class="GtkVBox" id="main-vbox">
        <property name="visible">True</property>
        <child>
          <object class="GtkHBox" id="main-hbox">
            <property name="visible">True</property>
            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
            <child>
              <object class="GtkStatusbar" id="main-statusbar">
                <property name="visible">True</property>
                <property name="has_resize_grip">False</property>
              </object>
            </child>
            <child>
              <object class="GtkProgressBar" id="main-progress-bar">
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
            <property name="pack_type">GTK_PACK_END</property>
            <property name="position">1</property>
          </packing>
        </child>
        <child>
          <object class="GtkVBox" id="main-contents">
            <property name="visible">True</property>
            <property name="border_width">12</property>
            <property name="spacing">12</property>
            <child>
              <object class="GtkHBox" id="main-top-hbox">
                <property name="visible">True</property>
                <property name="spacing">12</property>
                <child>
                  <object class="GtkHButtonBox" id="main-top-button-box">
                    <property name="visible">True</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="spacing">3</property>
                    <property name="homogeneous">True</property>
                    <property name="layout_style">GTK_BUTTONBOX_START</property>
                    <child>
                      <object class="GtkToggleButton" id="main-view-button-0">
                        <property name="visible">True</property>
                        <property name="can_focus">True</property>
                        <property name="receives_default">True</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="tooltip-text" translatable="yes">Show all available games</property>
                      </object>
                    </child>
                    <child>
                      <object class="GtkToggleButton" id="main-view-button-1">
                        <property name="visible">True</property>
                        <property name="can_focus">True</property>
                        <property name="receives_default">True</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="tooltip-text" translatable="yes">Only show my favorite games</property>
                      </object>
                      <packing>
                        <property name="position">1</property>
                      </packing>
                    </child>
                    <child>
                      <object class="GtkToggleButton" id="main-view-button-2">
                        <property name="visible">True</property>
                        <property name="can_focus">True</property>
                        <property name="receives_default">True</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="tooltip-text" translatable="yes">Show my search results</property>
                      </object>
                      <packing>
                        <property name="position">2</property>
                      </packing>
                    </child>
                  </object>
                </child>
                <child>
                  <object class="GvaMuteButton" id="main-mute-button">
                    <property name="visible">True</property>
                    <property name="receives_default">True</property>
                    <property name="has_tooltip">True</property>
                    <property name="tooltip_markup">In-game sound is enabled</property>
                    <property name="tooltip_text">In-game sound is enabled</property>
                    <property name="relief">GTK_RELIEF_NONE</property>
                    <child>
                      <placeholder/>
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
              </packing>
            </child>
            <child>
              <object class="GtkVBox" id="main-center-vbox">
                <property name="visible">True</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="spacing">2</property>
                <child>
                  <object class="GtkScrolledWindow" id="main-scrolled-window">
                    <property name="visible">True</property>
                    <property name="can_focus">True</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="hscrollbar_policy">GTK_POLICY_AUTOMATIC</property>
                    <property name="vscrollbar_policy">GTK_POLICY_AUTOMATIC</property>
                    <property name="shadow_type">GTK_SHADOW_IN</property>
                    <child>
                      <object class="GtkTreeView" id="main-tree-view">
                        <property name="visible">True</property>
                        <property name="sensitive">False</property>
                        <property name="can_focus">True</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="has_tooltip">True</property>
                        <property name="rules_hint">True</property>
                        <property name="show_expanders">False</property>
                        <signal handler="gva_tree_view_popup_menu_cb" name="popup_menu"/>
                        <signal handler="gva_tree_view_button_press_event_cb" name="button_press_event"/>
                        <signal handler="gva_tree_view_query_tooltip_cb" name="query_tooltip"/>
                        <signal handler="gva_tree_view_row_activated_cb" name="row_activated"/>
                        <signal handler="gva_columns_save" name="columns_changed"/>
                      </object>
                    </child>
                  </object>
                </child>
                <child>
                  <object class="GtkHBox" id="main-search-hbox">
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="has_tooltip">True</property>
                    <property name="spacing">6</property>
                    <signal handler="gva_main_search_query_tooltip_cb" name="query_tooltip"/>
                    <child>
                      <object class="GtkLabel" id="main-search-label">
                        <property name="visible">True</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="xalign">0</property>
                        <property comments="This label precedes the search entry. e.g. Search for: Pac-Man" name="label" translatable="yes">Search for:</property>
                      </object>
                      <packing>
                        <property name="expand">False</property>
                        <property name="fill">False</property>
                      </packing>
                    </child>
                    <child>
                      <object class="GtkEntry" id="main-search-entry">
                        <property name="visible">True</property>
                        <property name="can_focus">True</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <signal handler="gva_main_search_entry_activate_cb" name="activate"/>
                        <signal handler="gtk_widget_hide" name="remove_widget" object="main-search-hbox"/>
                        <signal handler="gva_main_search_entry_notify_cb" name="notify"/>
                      </object>
                      <packing>
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
                <property name="position">1</property>
              </packing>
            </child>
            <child>
              <object class="GtkHButtonBox" id="main-bottom-button-box">
                <property name="visible">True</property>
                <property name="spacing">12</property>
                <property name="layout_style">GTK_BUTTONBOX_END</property>
                <child>
                  <object class="GtkButton" id="main-properties-button">
                    <property name="visible">True</property>
                    <property name="can_focus">True</property>
                    <property name="can_default">True</property>
                    <property name="tooltip-text" translatable="yes">Show information about the selected game</property>
                    <property name="use_stock">True</property>
                  </object>
                  <packing>
                    <property name="position">1</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkButton" id="main-start-game-button">
                    <property name="visible">True</property>
                    <property name="sensitive">False</property>
                    <property name="can_focus">True</property>
                    <property name="has_focus">True</property>
                    <property name="can_default">True</property>
                    <property name="has_default">True</property>
                    <property name="tooltip-text" translatable="yes">Start the selected game</property>
                    <child>
                      <object class="GtkAlignment" id="main-start-game-button-alignment">
                        <property name="visible">True</property>
                        <property name="xscale">0</property>
                        <property name="yscale">0</property>
                        <child>
                          <object class="GtkHBox" id="main-start-game-button-hbox">
                            <property name="visible">True</property>
                            <property name="spacing">2</property>
                            <child>
                              <object class="GtkImage" id="main-start-game-button-image">
                                <property name="visible">True</property>
                                <property name="stock">gtk-execute</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="fill">False</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkLabel" id="main-start-game-button-label">
                                <property name="visible">True</property>
                                <property name="label" translatable="yes">_Start Game</property>
                                <property name="use_underline">True</property>
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
                    </child>
                  </object>
                  <packing>
                    <property name="position">2</property>
                  </packing>
                </child>
              </object>
              <packing>
                <property name="expand">False</property>
                <property name="position">2</property>
              </packing>
            </child>
          </object>
          <packing>
            <property name="pack_type">GTK_PACK_END</property>
          </packing>
        </child>
      </object>
    </child>
  </object>
  <object class="GtkWindow" id="play-back-window">
    <property name="title" translatable="yes">Recorded Games</property>
    <property name="window_position">GTK_WIN_POS_CENTER_ON_PARENT</property>
    <property name="default_width">550</property>
    <property name="default_height">250</property>
    <property name="destroy_with_parent">True</property>
    <property name="transient_for">main-window</property>
    <signal handler="gva_play_back_window_hide_cb" name="hide"/>
    <signal handler="gtk_widget_hide_on_delete" name="delete_event"/>
    <child>
      <object class="GtkVBox" id="play-back-vbox">
        <property name="visible">True</property>
        <property name="border_width">12</property>
        <property name="spacing">12</property>
        <child>
          <object class="GtkScrolledWindow" id="play-back-scrolled-window">
            <property name="visible">True</property>
            <property name="can_focus">True</property>
            <property name="hscrollbar_policy">GTK_POLICY_AUTOMATIC</property>
            <property name="vscrollbar_policy">GTK_POLICY_AUTOMATIC</property>
            <property name="shadow_type">GTK_SHADOW_IN</property>
            <child>
              <object class="GtkTreeView" id="play-back-tree-view">
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="reorderable">True</property>
                <property name="rules_hint">True</property>
                <property name="show_expanders">False</property>
                <signal handler="gva_play_back_row_activated_cb" name="row_activated"/>
              </object>
            </child>
          </object>
        </child>
        <child>
          <object class="GtkHButtonBox" id="play-back-button-box">
            <property name="visible">True</property>
            <property name="spacing">12</property>
            <property name="layout_style">GTK_BUTTONBOX_END</property>
            <child>
              <object class="GtkButton" id="play-back-close-button">
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="can_default">True</property>
                <property name="tooltip-text" translatable="yes">Close this window</property>
                <property name="label">gtk-close</property>
                <property name="use_stock">True</property>
                <signal handler="gva_play_back_close_clicked_cb" name="clicked" object="play-back-window"/>
              </object>
            </child>
            <child>
              <object class="GtkButton" id="play-back-delete-button">
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="can_default">True</property>
                <property name="tooltip-text" translatable="yes">Delete the selected game recordings</property>
                <property name="label">gtk-delete</property>
                <property name="use_stock">True</property>
                <signal handler="gva_play_back_delete_clicked_cb" name="clicked" object="play-back-tree-view"/>
              </object>
              <packing>
                <property name="position">1</property>
              </packing>
            </child>
            <child>
              <object class="GtkButton" id="play-back-button">
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="has_focus">True</property>
                <property name="is_focus">True</property>
                <property name="can_default">True</property>
                <property name="has_default">True</property>
                <property name="receives_default">True</property>
                <property name="tooltip-text" translatable="yes">Play back the selected game recording</property>
                <signal handler="gva_play_back_clicked_cb" name="clicked"/>
                <child>
                  <object class="GtkAlignment" id="play-back-button-alignment">
                    <property name="visible">True</property>
                    <property name="xscale">0</property>
                    <property name="yscale">0</property>
                    <child>
                      <object class="GtkHBox" id="play-back-button-hbox">
                        <property name="visible">True</property>
                        <property name="spacing">2</property>
                        <child>
                          <object class="GtkImage" id="play-back-button-image">
                            <property name="visible">True</property>
                            <property name="stock">gtk-media-play</property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkLabel" id="play-back-button-label">
                            <property name="visible">True</property>
                            <property name="label" translatable="yes">Play _Back</property>
                            <property name="use_underline">True</property>
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
                </child>
              </object>
              <packing>
                <property name="position">2</property>
              </packing>
            </child>
          </object>
          <packing>
            <property name="expand">False</property>
            <property name="position">1</property>
          </packing>
        </child>
      </object>
    </child>
  </object>
  <object class="GtkWindow" id="preferences-window">
    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
    <property name="title" translatable="yes">Preferences</property>
    <property name="window_position">GTK_WIN_POS_CENTER_ON_PARENT</property>
    <property name="destroy_with_parent">True</property>
    <property name="transient_for">main-window</property>
    <signal handler="gtk_widget_hide_on_delete" name="delete_event"/>
    <child>
      <object class="GtkVBox" id="preferences-vbox">
        <property name="visible">True</property>
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
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="border_width">12</property>
                <property name="spacing">12</property>
                <child>
                  <object class="GtkVBox" id="preferences-general-vbox">
                    <property name="visible">True</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="spacing">6</property>
                    <child>
                      <object class="GtkLabel" id="preferences-general-label">
                        <property name="visible">True</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="xalign">0</property>
                        <property name="yalign">0</property>
                        <property name="label" translatable="yes">&lt;b&gt;General&lt;/b&gt;</property>
                        <property name="use_markup">True</property>
                      </object>
                      <packing>
                        <property name="expand">False</property>
                      </packing>
                    </child>
                    <child>
                      <object class="GtkAlignment" id="preferences-general-alignment">
                        <property name="visible">True</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="left_padding">12</property>
                        <child>
                          <object class="GtkVBox" id="preferences-general-inner-vbox">
                            <property name="visible">True</property>
                            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkCheckButton" id="preferences-full-screen">
                                <property name="visible">True</property>
                                <property name="can_focus">True</property>
                                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                                <property name="label">(full-screen) GtkUIManager supplies the label</property>
                                <property name="use_underline">True</property>
                                <property name="draw_indicator">True</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkCheckButton" id="preferences-auto-save">
                                <property name="visible">True</property>
                                <property name="can_focus">True</property>
                                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                                <property name="label">(auto-save) GtkUIManager supplies the label</property>
                                <property name="use_underline">True</property>
                                <property name="draw_indicator">True</property>
                              </object>
                              <packing>
                                <property name="position">1</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkAlignment" id="preferences-auto-save-note-alignment">
                                <property name="visible">True</property>
                                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                                <property name="left_padding">24</property>
                                <child>
                                  <object class="GtkLabel" id="preferences-auto-save-info-label">
                                    <property name="visible">True</property>
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
                  </packing>
                </child>
                <child>
                  <object class="GtkVBox" id="preferences-game-list-vbox">
                    <property name="visible">True</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="spacing">6</property>
                    <child>
                      <object class="GtkLabel" id="preferences-game-list-label">
                        <property name="visible">True</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="xalign">0</property>
                        <property name="label" translatable="yes">&lt;b&gt;Game List&lt;/b&gt;</property>
                        <property name="use_markup">True</property>
                      </object>
                      <packing>
                        <property name="expand">False</property>
                      </packing>
                    </child>
                    <child>
                      <object class="GtkAlignment" id="preferences-game-list-alignment">
                        <property name="visible">True</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="left_padding">12</property>
                        <child>
                          <object class="GtkVBox" id="preferences-game-list-inner-vbox">
                            <property name="visible">True</property>
                            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkCheckButton" id="preferences-show-clones">
                                <property name="visible">True</property>
                                <property name="can_focus">True</property>
                                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                                <property name="label">(show-clones) GtkUIManager supplies the label</property>
                                <property name="draw_indicator">True</property>
                              </object>
                            </child>
                          </object>
                        </child>
                      </object>
                      <packing>
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
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="border_width">12</property>
                <property name="spacing">6</property>
                <child>
                  <object class="GtkLabel" id="preferences-columns-label">
                    <property name="visible">True</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="xalign">0</property>
                    <property name="label" translatable="yes">&lt;b&gt;Columns&lt;/b&gt;</property>
                    <property name="use_markup">True</property>
                  </object>
                  <packing>
                    <property name="expand">False</property>
                    <property name="fill">False</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkAlignment" id="preferences-columns-alignment">
                    <property name="visible">True</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="left_padding">12</property>
                    <child>
                      <object class="GtkVBox" id="preferences-columns-inner-vbox">
                        <property name="visible">True</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="spacing">6</property>
                        <child>
                          <object class="GtkLabel" id="preferences-columns-instructions1">
                            <property name="visible">True</property>
                            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">Choose the order of information to appear in the game list.</property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GvaColumnManager" id="preferences-column-manager">
                            <property name="visible">True</property>
                            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                            <property name="spacing">6</property>
                            <property name="managed_view">main-tree-view</property>
                          </object>
                          <packing>
                            <property name="position">1</property>
                          </packing>
                        </child>
                      </object>
                    </child>
                  </object>
                  <packing>
                    <property name="position">1</property>
                  </packing>
                </child>
              </object>
            </child>
            <child type="tab">
              <object class="GtkLabel" id="preferences-columns-tab-label">
                <property name="visible">True</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="label" translatable="yes">Columns</property>
              </object>
              <packing>
                <property name="position">1</property>
                <property name="tab_fill">False</property>
              </packing>
            </child>
          </object>
        </child>
        <child>
          <object class="GtkHButtonBox" id="preferences-button-box">
            <property name="visible">True</property>
            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
            <property name="spacing">12</property>
            <property name="layout_style">GTK_BUTTONBOX_END</property>
            <child>
              <object class="GtkButton" id="preferences-close-button">
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="has_focus">True</property>
                <property name="is_focus">True</property>
                <property name="can_default">True</property>
                <property name="has_default">True</property>
                <property name="receives_default">True</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="tooltip-text" translatable="yes">Close this window</property>
                <property name="label">gtk-close</property>
                <property name="use_stock">True</property>
                <signal handler="gva_preferences_close_clicked_cb" name="clicked" object="preferences-window"/>
              </object>
            </child>
          </object>
          <packing>
            <property name="expand">False</property>
            <property name="position">1</property>
          </packing>
        </child>
      </object>
    </child>
  </object>
  <object class="GtkWindow" id="properties-window">
    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
    <property name="border_width">12</property>
    <property name="title" translatable="yes">Properties</property>
    <property name="window_position">GTK_WIN_POS_CENTER_ON_PARENT</property>
    <property name="default_width">500</property>
    <property name="default_height">400</property>
    <property name="destroy_with_parent">True</property>
    <property name="transient_for">main-window</property>
    <signal handler="gtk_widget_hide_on_delete" name="delete_event"/>
    <signal handler="gva_properties_show_cb" name="show"/>
    <child>
      <object class="GtkVBox" id="properties-vbox">
        <property name="visible">True</property>
        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
        <property name="spacing">12</property>
        <child>
          <object class="GtkTable" id="properties-table">
            <property name="visible">True</property>
            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
            <property name="n_rows">2</property>
            <property name="n_columns">2</property>
            <property name="column_spacing">12</property>
            <child>
              <placeholder/>
            </child>
            <child>
              <object class="GtkHBox" id="properties-hbox">
                <property name="visible">True</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="spacing">3</property>
                <property name="homogeneous">True</property>
                <child>
                  <object class="GtkButton" id="properties-back-button">
                    <property name="visible">True</property>
                    <property name="can_focus">True</property>
                    <property name="receives_default">True</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="tooltip-text" translatable="yes">Show previous game</property>
                    <child>
                      <object class="GtkImage" id="properties-back-button-image">
                        <property name="visible">True</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="stock">gtk-go-back</property>
                      </object>
                    </child>
                  </object>
                  <packing>
                    <property name="expand">False</property>
                    <property name="fill">False</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkButton" id="properties-forward-button">
                    <property name="visible">True</property>
                    <property name="can_focus">True</property>
                    <property name="receives_default">True</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="tooltip-text" translatable="yes">Show next game</property>
                    <child>
                      <object class="GtkImage" id="properties-forward-button-image">
                        <property name="visible">True</property>
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
                <property name="x_options"/>
                <property name="y_options"/>
              </packing>
            </child>
            <child>
              <object class="GtkLabel" id="properties-header">
                <property name="visible">True</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="xalign">0</property>
                <property name="yalign">0</property>
                <property name="label">&lt;big&gt;&lt;b&gt;Game Description&lt;/b&gt;&lt;/big&gt;
Manufacturer, Year</property>
                <property name="use_markup">True</property>
                <property name="ellipsize">PANGO_ELLIPSIZE_END</property>
              </object>
              <packing>
                <property name="bottom_attach">2</property>
              </packing>
            </child>
          </object>
          <packing>
            <property name="expand">False</property>
            <property name="fill">False</property>
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
                <property name="can_focus">True</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="show_tabs">False</property>
                <property name="show_border">False</property>
                <child>
                  <object class="GtkScrolledWindow" id="properties-history-scrolled-window">
                    <property name="visible">True</property>
                    <property name="can_focus">True</property>
                    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                    <property name="border_width">6</property>
                    <property name="hscrollbar_policy">GTK_POLICY_NEVER</property>
                    <property name="vscrollbar_policy">GTK_POLICY_AUTOMATIC</property>
                    <property name="shadow_type">GTK_SHADOW_IN</property>
                    <child>
                      <object class="GtkTextView" id="properties-history-text-view">
                        <property name="visible">True</property>
                        <property name="can_focus">True</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="editable">False</property>
                        <property name="wrap_mode">GTK_WRAP_WORD</property>
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
                <property name="can_focus">True</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="show_tabs">False</property>
                <property name="show_border">False</property>
                <child>
                  <object class="GtkImage" id="properties-gallery-image">
                    <property name="visible">True</property>
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
            </child>
            <child type="tab">
              <object class="GtkLabel" id="properties-gallery-tab">
                <property name="visible">True</property>
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
                <property name="hscrollbar_policy">GTK_POLICY_NEVER</property>
                <property name="vscrollbar_policy">GTK_POLICY_AUTOMATIC</property>
                <child>
                  <object class="GtkViewport" id="properties-technical-viewport">
                    <property name="visible">True</property>
                    <property name="can_focus">True</property>
                    <property name="resize_mode">GTK_RESIZE_QUEUE</property>
                    <child>
                      <object class="GtkVBox" id="properties-technical-vbox">
                        <property name="visible">True</property>
                        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                        <property name="border_width">12</property>
                        <property name="spacing">12</property>
                        <child>
                          <object class="GtkFrame" id="properties-status-frame">
                            <property name="visible">True</property>
                            <property name="label_xalign">0</property>
                            <property name="label_yalign">0</property>
                            <property name="shadow_type">GTK_SHADOW_OUT</property>
                            <child>
                              <object class="GtkAlignment" id="properties-status-frame-alignment">
                                <property name="visible">True</property>
                                <child>
                                  <object class="GtkEventBox" id="properties-status-event-box">
                                    <property name="visible">True</property>
                                    <child>
                                      <object class="GtkHBox" id="properties-status-hbox">
                                        <property name="visible">True</property>
                                        <property name="border_width">6</property>
                                        <property name="spacing">12</property>
                                        <child>
                                          <object class="GtkImage" id="properties-status-image">
                                            <property name="visible">True</property>
                                            <property name="yalign">0</property>
                                            <property name="stock">gtk-dialog-warning</property>
                                            <property name="icon_size">5</property>
                                          </object>
                                          <packing>
                                            <property name="expand">False</property>
                                            <property name="fill">False</property>
                                          </packing>
                                        </child>
                                        <child>
                                          <object class="GtkVBox" id="properties-status-vbox">
                                            <property name="visible">True</property>
                                            <property name="spacing">6</property>
                                            <child>
                                              <object class="GtkLabel" id="properties-status-header">
                                                <property name="visible">True</property>
                                                <property name="xalign">0</property>
                                                <property name="label" translatable="yes">&lt;b&gt;There are known problems with this game:&lt;/b&gt;</property>
                                                <property name="use_markup">True</property>
                                                <property name="ellipsize">PANGO_ELLIPSIZE_END</property>
                                              </object>
                                              <packing>
                                                <property name="expand">False</property>
                                                <property name="fill">False</property>
                                              </packing>
                                            </child>
                                            <child>
                                              <object class="GtkVBox" id="properties-status-detail-vbox">
                                                <property name="visible">True</property>
                                                <child>
                                                  <object class="GtkLabel" id="properties-imperfect-color-label">
                                                    <property name="visible">True</property>
                                                    <property name="xalign">0</property>
                                                    <property name="label" translatable="yes">&#x2022; The colors aren't 100% accurate.</property>
                                                    <property name="ellipsize">PANGO_ELLIPSIZE_END</property>
                                                  </object>
                                                </child>
                                                <child>
                                                  <object class="GtkLabel" id="properties-preliminary-color-label">
                                                    <property name="visible">True</property>
                                                    <property name="xalign">0</property>
                                                    <property name="label" translatable="yes">&#x2022; The colors are completely wrong.</property>
                                                    <property name="ellipsize">PANGO_ELLIPSIZE_END</property>
                                                  </object>
                                                  <packing>
                                                    <property name="position">1</property>
                                                  </packing>
                                                </child>
                                                <child>
                                                  <object class="GtkLabel" id="properties-imperfect-graphic-label">
                                                    <property name="visible">True</property>
                                                    <property name="xalign">0</property>
                                                    <property name="label" translatable="yes">&#x2022; The video emulation isn't 100% accurate.</property>
                                                    <property name="ellipsize">PANGO_ELLIPSIZE_END</property>
                                                  </object>
                                                  <packing>
                                                    <property name="position">2</property>
                                                  </packing>
                                                </child>
                                                <child>
                                                  <object class="GtkLabel" id="properties-imperfect-sound-label">
                                                    <property name="visible">True</property>
                                                    <property name="xalign">0</property>
                                                    <property name="label" translatable="yes">&#x2022; The sound emulation isn't 100% accurate.</property>
                                                    <property name="ellipsize">PANGO_ELLIPSIZE_END</property>
                                                  </object>
                                                  <packing>
                                                    <property name="position">3</property>
                                                  </packing>
                                                </child>
                                                <child>
                                                  <object class="GtkLabel" id="properties-preliminary-sound-label">
                                                    <property name="visible">True</property>
                                                    <property name="xalign">0</property>
                                                    <property name="label" translatable="yes">&#x2022; The game lacks sound.</property>
                                                    <property name="ellipsize">PANGO_ELLIPSIZE_END</property>
                                                  </object>
                                                  <packing>
                                                    <property name="position">4</property>
                                                  </packing>
                                                </child>
                                                <child>
                                                  <object class="GtkLabel" id="properties-preliminary-cocktail-label">
                                                    <property name="visible">True</property>
                                                    <property name="xalign">0</property>
                                                    <property name="label" translatable="yes">&#x2022; Screen flipping in cocktail mode is not supported.</property>
                                                    <property name="ellipsize">PANGO_ELLIPSIZE_END</property>
                                                  </object>
                                                  <packing>
                                                    <property name="position">5</property>
                                                  </packing>
                                                </child>
                                                <child>
                                                  <object class="GtkLabel" id="properties-preliminary-emulation-label">
                                                    <property name="visible">True</property>
                                                    <property name="xalign">0</property>
                                                    <property name="label" translatable="yes">&#x2022; &lt;b&gt;THIS GAME DOESN'T WORK.&lt;/b&gt;</property>
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
                                                    <property name="xalign">0</property>
                                                    <property name="label" translatable="yes">&#x2022; The game has protection which isn't fully emulated.</property>
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
                              </object>
                            </child>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkVBox" id="properties-bios-vbox">
                            <property name="visible">True</property>
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkLabel" id="properties-bios-header">
                                <property name="visible">True</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">&lt;b&gt;BIOS&lt;/b&gt;</property>
                                <property name="use_markup">True</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="fill">False</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkAlignment" id="properties-bios-alignment">
                                <property name="visible">True</property>
                                <property name="left_padding">12</property>
                                <child>
                                  <object class="GtkLabel" id="properties-bios-label">
                                    <property name="visible">True</property>
                                    <property name="xalign">0</property>
                                    <property name="label">BIOS System Name</property>
                                  </object>
                                </child>
                              </object>
                              <packing>
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
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkLabel" id="properties-cpu-header">
                                <property name="visible">True</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">&lt;b&gt;CPU&lt;/b&gt;</property>
                                <property name="use_markup">True</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="fill">False</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkAlignment" id="properties-cpu-alignment">
                                <property name="visible">True</property>
                                <property name="left_padding">12</property>
                                <child>
                                  <object class="GtkVBox" id="properties-cpu-inner-vbox">
                                    <property name="visible">True</property>
                                    <property name="spacing">6</property>
                                    <child>
                                      <object class="GtkLabel" id="properties-cpu0-label">
                                        <property name="visible">True</property>
                                        <property name="xalign">0</property>
                                        <property name="label">CPU #0 Information</property>
                                      </object>
                                      <packing>
                                        <property name="expand">False</property>
                                        <property name="fill">False</property>
                                      </packing>
                                    </child>
                                    <child>
                                      <object class="GtkLabel" id="properties-cpu1-label">
                                        <property name="visible">True</property>
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
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkLabel" id="properties-sound-header">
                                <property name="visible">True</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">&lt;b&gt;Sound&lt;/b&gt;</property>
                                <property name="use_markup">True</property>
                              </object>
                            </child>
                            <child>
                              <object class="GtkAlignment" id="properties-sound-alignment">
                                <property name="visible">True</property>
                                <property name="left_padding">12</property>
                                <child>
                                  <object class="GtkVBox" id="properties-sound-inner-vbox">
                                    <property name="visible">True</property>
                                    <property name="spacing">6</property>
                                    <child>
                                      <object class="GtkLabel" id="properties-sound0-label">
                                        <property name="visible">True</property>
                                        <property name="xalign">0</property>
                                        <property name="label">Sound #0 Information</property>
                                      </object>
                                      <packing>
                                        <property name="expand">False</property>
                                        <property name="fill">False</property>
                                      </packing>
                                    </child>
                                    <child>
                                      <object class="GtkLabel" id="properties-sound1-label">
                                        <property name="visible">True</property>
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
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkLabel" id="properties-video-header">
                                <property name="visible">True</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">&lt;b&gt;Video&lt;/b&gt;</property>
                                <property name="use_markup">True</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="fill">False</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkAlignment" id="properties-video-alignment">
                                <property name="visible">True</property>
                                <property name="left_padding">12</property>
                                <child>
                                  <object class="GtkVBox" id="properties-video-inner-vbox">
                                    <property name="visible">True</property>
                                    <property name="spacing">6</property>
                                    <child>
                                      <object class="GtkLabel" id="properties-video0-label">
                                        <property name="visible">True</property>
                                        <property name="xalign">0</property>
                                        <property name="label">Video #0 Information</property>
                                      </object>
                                      <packing>
                                        <property name="expand">False</property>
                                        <property name="fill">False</property>
                                      </packing>
                                    </child>
                                    <child>
                                      <object class="GtkLabel" id="properties-video1-label">
                                        <property name="visible">True</property>
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
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkLabel" id="properties-original-header">
                                <property name="visible">True</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">&lt;b&gt;Original Version&lt;/b&gt;</property>
                                <property name="use_markup">True</property>
                              </object>
                            </child>
                            <child>
                              <object class="GtkAlignment" id="properties-original-alignment">
                                <property name="visible">True</property>
                                <property name="left_padding">12</property>
                                <child>
                                  <object class="GtkVBox" id="properties-original-links">
                                    <property name="visible">True</property>
                                    <property name="spacing">6</property>
                                    <child>
                                      <placeholder/>
                                    </child>
                                  </object>
                                </child>
                              </object>
                              <packing>
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
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkLabel" id="properties-alternate-header">
                                <property name="visible">True</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">&lt;b&gt;Alternate Versions&lt;/b&gt;</property>
                                <property name="use_markup">True</property>
                              </object>
                            </child>
                            <child>
                              <object class="GtkAlignment" id="properties-alternate-alignment">
                                <property name="visible">True</property>
                                <property name="left_padding">12</property>
                                <child>
                                  <object class="GtkVBox" id="properties-alternate-links">
                                    <property name="visible">True</property>
                                    <property name="spacing">6</property>
                                    <child>
                                      <placeholder/>
                                    </child>
                                  </object>
                                </child>
                              </object>
                              <packing>
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
            </child>
            <child type="tab">
              <object class="GtkLabel" id="properties-technical-tab">
                <property name="visible">True</property>
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
            <property name="position">1</property>
          </packing>
        </child>
        <child>
          <object class="GtkHButtonBox" id="properties-button-box">
            <property name="visible">True</property>
            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
            <property name="layout_style">GTK_BUTTONBOX_END</property>
            <child>
              <object class="GtkButton" id="properties-close-button">
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="has_focus">True</property>
                <property name="is_focus">True</property>
                <property name="can_default">True</property>
                <property name="has_default">True</property>
                <property name="receives_default">True</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="tooltip-text" translatable="yes">Close this window</property>
                <property name="label">gtk-close</property>
                <property name="use_stock">True</property>
                <signal handler="gva_properties_close_clicked_cb" name="clicked" object="properties-window"/>
              </object>
              <packing>
                <property name="expand">False</property>
                <property name="fill">False</property>
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
  <object class="GtkWindow" id="audit-window">
    <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
    <property name="title" translatable="yes"> </property>
    <property name="resizable">False</property>
    <property name="window_position">GTK_WIN_POS_CENTER_ON_PARENT</property>
    <property name="default_width">400</property>
    <property name="destroy_with_parent">True</property>
    <property name="type_hint">GDK_WINDOW_TYPE_HINT_DIALOG</property>
    <property name="urgency_hint">True</property>
    <property name="transient_for">main-window</property>
    <child>
      <object class="GtkTable" id="audit-table">
        <property name="visible">True</property>
        <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
        <property name="border_width">12</property>
        <property name="n_rows">5</property>
        <property name="n_columns">2</property>
        <property name="column_spacing">12</property>
        <property name="row_spacing">12</property>
        <child>
          <object class="GtkHBox" id="audit-info-hbox">
            <property name="visible">True</property>
            <property name="spacing">6</property>
            <child>
              <object class="GtkImage" id="audit-info-image">
                <property name="visible">True</property>
                <property name="yalign">0</property>
                <property name="stock">gtk-dialog-info</property>
              </object>
              <packing>
                <property name="expand">False</property>
                <property name="fill">False</property>
              </packing>
            </child>
            <child>
              <object class="GtkLabel" id="audit-info-label">
                <property name="visible">True</property>
                <property name="xalign">0</property>
                <property name="label" translatable="yes">&lt;small&gt;&lt;i&gt;If these games worked with previous versions of MAME, it is likely their ROM requirements have changed.  If so, you will need to acquire more recent ROM files.&lt;/i&gt;&lt;/small&gt;</property>
                <property name="use_markup">True</property>
                <property name="wrap">True</property>
              </object>
              <packing>
                <property name="position">1</property>
              </packing>
            </child>
          </object>
          <packing>
            <property name="left_attach">1</property>
            <property name="right_attach">2</property>
            <property name="top_attach">2</property>
            <property name="bottom_attach">3</property>
            <property name="y_options"/>
          </packing>
        </child>
        <child>
          <object class="GtkScrolledWindow" id="audit-scrolled-window">
            <property name="height_request">200</property>
            <property name="visible">True</property>
            <property name="can_focus">True</property>
            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
            <property name="hscrollbar_policy">GTK_POLICY_AUTOMATIC</property>
            <property name="vscrollbar_policy">GTK_POLICY_AUTOMATIC</property>
            <property name="shadow_type">GTK_SHADOW_IN</property>
            <child>
              <object class="GtkTreeView" id="audit-tree-view">
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="headers_visible">False</property>
              </object>
            </child>
          </object>
          <packing>
            <property name="left_attach">1</property>
            <property name="right_attach">2</property>
            <property name="top_attach">3</property>
            <property name="bottom_attach">4</property>
          </packing>
        </child>
        <child>
          <object class="GtkHButtonBox" id="audit-button-box">
            <property name="visible">True</property>
            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
            <property name="spacing">12</property>
            <property name="homogeneous">True</property>
            <property name="layout_style">GTK_BUTTONBOX_END</property>
            <child>
              <object class="GtkButton" id="audit-save-button">
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="receives_default">True</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="tooltip-text" translatable="yes">Save ROM errors to a file</property>
                <property name="use_stock">True</property>
              </object>
            </child>
            <child>
              <object class="GtkButton" id="audit-close-button">
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="has_focus">True</property>
                <property name="is_focus">True</property>
                <property name="can_default">True</property>
                <property name="has_default">True</property>
                <property name="receives_default">True</property>
                <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
                <property name="tooltip-text" translatable="yes">Close this window</property>
                <property name="label" translatable="yes">gtk-close</property>
                <property name="use_stock">True</property>
                <signal handler="gtk_widget_hide" name="clicked" object="audit-window"/>
              </object>
              <packing>
                <property name="position">1</property>
              </packing>
            </child>
          </object>
          <packing>
            <property name="right_attach">2</property>
            <property name="top_attach">4</property>
            <property name="bottom_attach">5</property>
            <property name="y_options"/>
          </packing>
        </child>
        <child>
          <object class="GtkImage" id="audit-image">
            <property name="visible">True</property>
            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
            <property name="yalign">0</property>
            <property name="stock">gtk-dialog-warning</property>
            <property name="icon_size">6</property>
          </object>
          <packing>
            <property name="bottom_attach">4</property>
            <property name="x_options"/>
          </packing>
        </child>
        <child>
          <object class="GtkLabel" id="audit-title">
            <property name="visible">True</property>
            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
            <property name="xalign">0</property>
            <property name="yalign">0</property>
            <property name="label" translatable="yes">&lt;big&gt;&lt;b&gt;Errors detected in ROM files&lt;/b&gt;&lt;/big&gt;</property>
            <property name="use_markup">True</property>
            <property name="wrap">True</property>
          </object>
          <packing>
            <property name="left_attach">1</property>
            <property name="right_attach">2</property>
            <property name="y_options"/>
          </packing>
        </child>
        <child>
          <object class="GtkLabel" id="audit-verbage">
            <property name="visible">True</property>
            <property name="events">GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK</property>
            <property name="xalign">0</property>
            <property name="label" translatable="yes">An audit of the available games has detected errors in some of the ROM files. The following games will not be available in GNOME Video Arcade.</property>
            <property name="use_markup">True</property>
            <property name="wrap">True</property>
          </object>
          <packing>
            <property name="left_attach">1</property>
            <property name="right_attach">2</property>
            <property name="top_attach">1</property>
            <property name="bottom_attach">2</property>
            <property name="y_options"/>
          </packing>
        </child>
      </object>
    </child>
  </object>
</interface>
