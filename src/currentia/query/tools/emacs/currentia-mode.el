;;; currentia-mode.el --- A major-mode for editing currentia queries

;; Copyright (C) 2012  Masafumi Oyamada

;; Author: Masafumi Oyamada <masa@kde.cs.tsukuba.ac.jp>
;; Keywords:

;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.

;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <http://www.gnu.org/licenses/>.

;;; Commentary:

;;; Code:

(require 'font-lock)
(require 'cc-mode)
(require 'make-regexp)

;; Syntax highlight support

(setq currentia-types
      (make-regexp (list "int"
                         "float"
                         "string"
                         "blob")))

(setq currentia-keywords
      (make-regexp (mapcar (lambda (keyword) (concat "\\_<" keyword "\\_>"))
                           (list
                            "and"
                            "combine"
                            "day"
                            "elect"
                            "from"
                            "hour"
                            "mean"
                            "min"
                            "msec"
                            "not"
                            "or"
                            "project\\(ion\\)?"
                            "recent"
                            "relation"
                            "rows"
                            "sec"
                            "select\\(ion\\)?"
                            "slide"
                            "stream"
                            "sum"
                            "where"))))

(setq currentia-comment "#.*$")

(setq currentia-font-lock-keywords
  `((,currentia-comment . font-lock-comment-face)
    (,currentia-keywords . font-lock-keyword-face)
    (,currentia-types . font-lock-type-face)
    ("\\(stream\\|relation\\)[ \t]*\\([a-zA-Z0-9_]+\\)" 2 font-lock-variable-name-face)))

(define-derived-mode currentia-mode text-mode
  "Currentia"
  "Major mode for editing currentia queries"
  (setq mode-name "Currentia")
  (setq major-mode 'currentia-mode)

  (set (make-local-variable 'comment-start) "# ")

  ;; Syntax highlight
  (font-lock-add-keywords nil currentia-font-lock-keywords))

(provide 'currentia-mode)
;;; currentia-mode.el ends here
