use vars qw(%result_texis %result_texts %result_trees %result_errors 
   %result_indices %result_sectioning %result_nodes %result_menus
   %result_floats %result_converted %result_converted_errors 
   %result_elements %result_directions_text %result_indices_sort_strings);

use utf8;

$result_trees{'text_arguments'} = {
  'contents' => [
    {
      'contents' => [
        {
          'args' => [
            {
              'text' => 'nospace',
              'type' => 'macro_name'
            },
            {
              'text' => 'one',
              'type' => 'macro_arg'
            },
            {
              'text' => 'two',
              'type' => 'macro_arg'
            },
            {
              'text' => 'three',
              'type' => 'macro_arg'
            }
          ],
          'cmdname' => 'linemacro',
          'contents' => [
            {
              'text' => '\\three\\@bullet{}\\two\\@minus{}\\one\\
',
              'type' => 'raw'
            },
            {
              'args' => [
                {
                  'contents' => [
                    {
                      'text' => 'linemacro'
                    }
                  ],
                  'info' => {
                    'spaces_after_argument' => {
                      'text' => '
'
                    }
                  },
                  'type' => 'line_arg'
                }
              ],
              'cmdname' => 'end',
              'extra' => {
                'text_arg' => 'linemacro'
              },
              'info' => {
                'spaces_before_argument' => {
                  'text' => ' '
                }
              },
              'source_info' => {
                'line_nr' => 3
              }
            }
          ],
          'info' => {
            'arg_line' => ' nospace{one, two, three}
'
          },
          'source_info' => {
            'line_nr' => 1
          }
        },
        {
          'text' => '
',
          'type' => 'empty_line'
        },
        {
          'contents' => [
            {
              'source_marks' => [
                {
                  'counter' => 1,
                  'element' => {
                    'args' => [
                      {
                        'contents' => [
                          {
                            'text' => 'X'
                          }
                        ],
                        'type' => 'line_arg'
                      },
                      {
                        'contents' => [
                          {
                            'text' => 'Y'
                          }
                        ],
                        'info' => {
                          'spaces_before_argument' => {
                            'text' => ' '
                          }
                        },
                        'type' => 'line_arg'
                      },
                      {
                        'contents' => [
                          {
                            'text' => 'Z'
                          }
                        ],
                        'info' => {
                          'spaces_before_argument' => {
                            'text' => ' '
                          }
                        },
                        'type' => 'line_arg'
                      }
                    ],
                    'info' => {
                      'command_name' => 'nospace',
                      'spaces_before_argument' => {
                        'text' => ' '
                      }
                    },
                    'type' => 'linemacro_call'
                  },
                  'sourcemark_type' => 'linemacro_expansion',
                  'status' => 'start'
                }
              ],
              'text' => 'Z'
            },
            {
              'args' => [
                {
                  'type' => 'brace_container'
                }
              ],
              'cmdname' => 'bullet',
              'source_info' => {
                'line_nr' => 5,
                'macro' => 'nospace'
              }
            },
            {
              'text' => 'Y'
            },
            {
              'args' => [
                {
                  'type' => 'brace_container'
                }
              ],
              'cmdname' => 'minus',
              'source_info' => {
                'line_nr' => 5,
                'macro' => 'nospace'
              }
            },
            {
              'source_marks' => [
                {
                  'counter' => 1,
                  'position' => 1,
                  'sourcemark_type' => 'linemacro_expansion',
                  'status' => 'end'
                }
              ],
              'text' => 'X
'
            }
          ],
          'type' => 'paragraph'
        }
      ],
      'type' => 'before_node_section'
    }
  ],
  'type' => 'document_root'
};

$result_texis{'text_arguments'} = '@linemacro nospace{one, two, three}
\\three\\@bullet{}\\two\\@minus{}\\one\\
@end linemacro

Z@bullet{}Y@minus{}X
';


$result_texts{'text_arguments'} = '
Z*Y-X
';

$result_errors{'text_arguments'} = [];


$result_floats{'text_arguments'} = {};


1;
